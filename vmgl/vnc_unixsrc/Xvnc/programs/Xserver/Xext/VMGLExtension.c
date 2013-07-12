/*
* Copyright (c) 2006-2007 H. Andres Lagar-Cavilla <andreslc@cs.toronto.edu>
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*
*   1. Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
*   2. Redistributions in binary form must reproduce the above
* copyright notice, this list of conditions and the following disclaimer
* in the documentation and/or other materials provided with the
* distribution.
*   3. The name of the author may not be used to endorse or promote
* products derived from this software without specific prior written
* permission.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/

/* vmgl */

#include "scrnintstr.h"
#include "extnsionst.h"
#include "vmgl_ext.h"
#include "vmgl_extproto.h"
#include "dixstruct.h"
#include "windowstr.h"

#include <errno.h>
#include <stdio.h>

#include "regionstr.h"


#include <sys/socket.h>
#include <sys/un.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <fcntl.h>


#include "scrnintstr.h"
#include "extnsionst.h"
#include "dixstruct.h"
#include "windowstr.h"

#include <X11/extensions/vmgl_ext.h>
#include <X11/extensions/vmgl_extproto.h>

#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>



/* List of windows to watch on behalf of a gl stub */
typedef struct GlWindowWatcher_struct {
    unsigned int XWindow;
    unsigned int glWindow;
    int sock;
    struct GlWindowWatcher_struct *next;
} GlWindowWatcher;

/* List of glStubs on behalf of whom we watch windows */
typedef struct GlStubWatcher_struct {
    int sock;
    unsigned int address;
    unsigned short port;
    GlWindowWatcher *WindowWatchersList;
    struct GlStubWatcher_struct *next;
} GlStubWatcher;

/* Globals */
GlStubWatcher *GlStubWatchersList;
GlWindowWatcher *GlWindowWatchersList;
void (*ClipNotify_wrap[MAXSCREENS])();
Bool (*DestroyWindow_wrap[MAXSCREENS])();
FILE *fp;

extern WindowPtr *WindowTable;

/* Helper */
char *IP2String(in_addr_t address) {
    struct in_addr tmp;
    tmp.s_addr= htonl(address);
    return ((char *) inet_ntoa(tmp));
}

/* This function does the communication with a gl Stub */
void SendWindowClipList(int sock, unsigned int glWindow, RegionRec *clipList,
		         int x, int y, unsigned int w, unsigned int h) {
    XVMGLWindowingCommand cmd;
    BoxPtr boxes;
    size_t writeLen;
    unsigned int len = REGION_NUM_RECTS(clipList);

    cmd.length = htonl(len);
    cmd.glWindow = htonl(glWindow);
    cmd.x = htonl(x);
    cmd.y = htonl(y);
    cmd.width = htonl(w);
    cmd.height = htonl(h);
    boxes = REGION_RECTS(clipList);

    writeLen = sizeof(XVMGLWindowingCommand);
    if (write(sock, (void *) &cmd, writeLen) != ((ssize_t) writeLen))
	fprintf(stderr, "Error writing on socket %d\n", sock);

    writeLen = sizeof(BoxRec)*len;
    if (writeLen)
	if (write(sock, (void *) boxes, writeLen) != ((ssize_t) writeLen))
	    fprintf(fp, "Error writing on socket %d\n", sock);	

#ifdef VERBOSE_VMGLEXT
    fp = fopen("/home/joshua/Desktop/xvnc_output.txt", "a");
    fprintf(stderr, "SendWindowClipList for window %u on socket %d: %d %d %u %u -> %u\n",
	    glWindow, sock, x, y, w, h, len);

    fprintf(fp, "SendWindowClipList for window %u on socket %d: %d %d %u %u -> %u\n",
	    glWindow, sock, x, y, w, h, len);
    fclose(fp);

#endif
}

/* Windowing functions hooks */
GlWindowWatcher *findWindow(unsigned int XWindow) {
    GlWindowWatcher *ptr;
    GlStubWatcher *srvPtr;

    for (srvPtr = GlStubWatchersList; srvPtr->next != NULL; srvPtr = srvPtr->next)
        for (ptr = srvPtr->next->WindowWatchersList; ptr->next != NULL; ptr = ptr->next)
            if (ptr->next->XWindow == XWindow)
                return ptr;

    return NULL;
}

Bool VMGLDestroyWindow(WindowPtr pWin) {
    GlWindowWatcher *tmp, *ptr = findWindow(pWin->drawable.id);
    BOOL retVal = TRUE;
    
    if (ptr) {
	fp = fopen("/home/joshua/Desktop/xvnc_output.txt", "a");
	fprintf(stderr, "Destroy %u XID %u\n", ptr->next->glWindow, (unsigned int) pWin->drawable.id);
	fprintf(fp, "Destroy %u XID %u\n", ptr->next->glWindow, (unsigned int) pWin->drawable.id);
	fclose(fp);
	tmp = ptr->next;
	ptr->next = tmp->next;
	xfree(tmp);
    }
    if (DestroyWindow_wrap[pWin->drawable.pScreen->myNum])
        retVal = (*DestroyWindow_wrap[pWin->drawable.pScreen->myNum])(pWin);
    /* Insist */
    screenInfo.screens[pWin->drawable.pScreen->myNum]->DestroyWindow = VMGLDestroyWindow;
    return retVal;
}

void VMGLClipNotify(WindowPtr pWin, int x, int y) {
    GlWindowWatcher *ptr = findWindow(pWin->drawable.id);
    if (ptr) 
	SendWindowClipList(ptr->next->sock, ptr->next->glWindow, &(pWin->clipList) /*RegionRec*/,
    		pWin->drawable.x, pWin->drawable.y, pWin->drawable.width, pWin->drawable.height);
    if (ClipNotify_wrap[pWin->drawable.pScreen->myNum])
            (*ClipNotify_wrap[pWin->drawable.pScreen->myNum])(pWin, x, y);                		
}

Bool RemoveGlStub(in_addr_t address, in_addr_t port) {    
    GlStubWatcher *tmp, *ptr;
    GlWindowWatcher *windowList, *windowTmp;
    for (tmp=GlStubWatchersList; tmp->next != NULL; tmp=tmp->next)
	if ((tmp->next->address == address)&&(tmp->next->port == port)) {
	    /* Removal */
	    ptr = tmp->next;
	    if (close(ptr->sock))
	    {
		fp = fopen("/home/joshua/Desktop/xvnc_output.txt", "a");
		fprintf(stderr, "Error %s closing socket to GlStub on address %s:%hu\n", strerror(errno), 
		        IP2String(address), (unsigned short) port);
	 	fprintf(fp, "Error %s closing socket to GlStub on address %s:%hu\n", strerror(errno), 
		        IP2String(address), (unsigned short) port);
		fclose(fp);
	    }

	    windowList = ptr->WindowWatchersList;
	    if (windowList->next)
	    {
		fp = fopen("/home/joshua/Desktop/xvnc_output.txt", "a");
		fprintf(stderr, "Removing GlStub with pending windows, address %s:%hu\n",
			IP2String(address), (unsigned short) port);
		fprintf(fp, "Removing GlStub with pending windows, address %s:%hu\n",
			IP2String(address), (unsigned short) port);
		fclose(fp);
	    }
	    while (windowList) {
		windowTmp = windowList;
		windowList = windowList->next;
		xfree(windowTmp);
	    }

	    tmp->next = ptr->next;
	    xfree(ptr);
	    fp = fopen("/home/joshua/Desktop/xvnc_output.txt", "a");
	    fprintf(stderr, "GlStub for address %s:%hu removed\n",
		    IP2String(address), (unsigned short) port);
	    fprintf(fp, "GlStub for address %s:%hu removed\n",
		    IP2String(address), (unsigned short) port);
	    fclose(fp);
	    return TRUE;
	}
    
    /* Never found it? */
    fp = fopen("/home/joshua/Desktop/xvnc_output.txt", "a");
    fprintf(stderr, "Couldn't find GlStub for address %s:%hu\n",
	    IP2String(address), (unsigned short) port);
    fprintf(fp, "Couldn't find GlStub for address %s:%hu\n",
	    IP2String(address), (unsigned short) port);
    fclose(fp);
    return FALSE;
}

/* These *are* the extension functions */
Bool AddGlStub(in_addr_t address, in_addr_t port) {
    GlStubWatcher *tmp;
    int sock;
    struct sockaddr_in addr;
    
    /* Establish connection with listening part */		
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	fp = fopen("/home/joshua/Desktop/xvnc_output.txt", "a");
	fprintf(stderr, "Failed to create socket: %s\n", strerror(errno));
	fprintf(fp, "Failed to create socket: %s\n", strerror(errno));
	fclose(fp);
	return FALSE;
    }
    memset((void *) &addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(address);
    if (connect(sock, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) < 0) {
	fp = fopen("/home/joshua/Desktop/xvnc_output.txt", "a");
	fprintf(stderr, "Failed to connect to address %s:%hu: %s\n",IP2String(address),
		(unsigned short) port, strerror(errno));
	fprintf(fp, "Failed to connect to address %s:%hu: %s\n",IP2String(address),
		(unsigned short) port, strerror(errno));
	fclose(fp);
	return FALSE;
    }

    /* Check it hasn't been already added */
    for (tmp=GlStubWatchersList; tmp->next != NULL; tmp=tmp->next)
	if ((tmp->next->address == address)&&(tmp->next->port == port)) {
	    fp = fopen("/home/joshua/Desktop/xvnc_output.txt", "a");
	    fprintf(stderr, "GlStub for address %s:%hu already added\n", 
		    IP2String(address), (unsigned short) port);
	    fprintf(fp, "GlStub for address %s:%hu already added\n", 
		    IP2String(address), (unsigned short) port);
	    fclose(fp);
	    close(tmp->next->sock);
	    tmp->next->sock = sock;
	    return TRUE;
	}
		
    /* Now add server to list */
    tmp = (GlStubWatcher *) xalloc(sizeof(GlStubWatcher));
    if (!tmp) {
	close(sock);
	return FALSE;
    }
    tmp->sock = sock;
    tmp->address = address;
    tmp->port = port;
    tmp->WindowWatchersList = (GlWindowWatcher *) xalloc(sizeof(GlWindowWatcher));
    if (!(tmp->WindowWatchersList)) {
	xfree(tmp);
	close(sock);
	return FALSE;
    }
    tmp->WindowWatchersList->next = NULL;
    tmp->next = GlStubWatchersList->next;
    GlStubWatchersList->next = tmp;
    fp = fopen("/home/joshua/Desktop/xvnc_output.txt", "a");
    fprintf(stderr, "Added GlStub for address %s:%hu\n", 
	    IP2String(address), (unsigned short) port);
    fprintf(fp, "Added GlStub for address %s:%hu\n", 
	    IP2String(address), (unsigned short) port);
    fclose(fp);
    return TRUE;
}

/* Ugly ugly ugly helper */
WindowPtr DepthSearchHelper(unsigned int XWindow, WindowPtr root) {
    WindowPtr pWin, tmp;
    
    for (pWin = root->firstChild; pWin; pWin = pWin->nextSib) 
	if (pWin->drawable.id == XWindow) 
	    return pWin;
	else 
	    if (tmp = DepthSearchHelper(XWindow, pWin))
		return tmp;

    return NULL;
}

WindowPtr DepthSearch(unsigned int XWindow) {
    int i;
    WindowPtr tmp;
    for (i=0;i<screenInfo.numScreens;i++)
	if (tmp = DepthSearchHelper(XWindow, WindowTable[i]))
	    return tmp;
}

Bool AddWindowWatch(unsigned int XWindow, unsigned int glWindow, 
		    in_addr_t address, in_port_t port)
{
    GlWindowWatcher *tmp, *ptr, *list;
    GlStubWatcher *srvTmp;
    WindowPtr thisWindow;
    
    if (!XWindow) {
	fp = fopen("/home/joshua/Desktop/xvnc_output.txt", "a");
	fprintf(stderr, "Meaningless XID to watch for address %s:%hu\n",
		IP2String(address), (unsigned short) port);
	fprintf(fp, "Meaningless XID to watch for address %s:%hu\n",
		IP2String(address), (unsigned short) port);
	fclose(fp);
	return FALSE;
    }
    
    /* First find the server */
    for (srvTmp=GlStubWatchersList; srvTmp->next != NULL; srvTmp=srvTmp->next)
	if ((srvTmp->next->address == address)&&(srvTmp->next->port == port)) {
		list = srvTmp->next->WindowWatchersList;
		break;
	}
    if (!srvTmp->next) {
	/* Didn't find server ?!?! */
	fp = fopen("/home/joshua/Desktop/xvnc_output.txt", "a");
	fprintf(stderr, "Couldn't find gl stub for address %s:%hu\n",
		IP2String(address), (unsigned short) port);
	fprintf(fp, "Couldn't find gl stub for address %s:%hu\n",
		IP2String(address), (unsigned short) port);
	fclose(fp);
	return FALSE;
    }
    
    /* Are we unwatching a window? */
    if (XWindow && !glWindow) {
	for (ptr=list; ptr->next != NULL; ptr=ptr->next) 
	    if (ptr->next->XWindow == XWindow) {
		fp = fopen("/home/joshua/Desktop/xvnc_output.txt", "a");
		fprintf(stderr,"Removing window %u for address %s:%hu\n",
			XWindow, IP2String(address), (unsigned short) port);
		fprintf(fp,"Removing window %u for address %s:%hu\n",
			XWindow, IP2String(address), (unsigned short) port);
		fclose(fp);
		tmp = ptr->next;
		ptr->next = tmp->next;
		xfree(tmp);
		/* Assume that once the list of windows is depleted
		   we can safely destroy the gl stub entry. NO!! */
/*		if (!(list->next))
		    return RemoveGlStub(srvTmp->next->address, srvTmp->next->port);
		else */
		    return TRUE;
	    }
	/* Could not find it */
	return FALSE;
    }
    
    for (ptr=list; ptr->next != NULL; ptr=ptr->next) 
	if (ptr->next->glWindow == glWindow) {
	    /* We have this, updating XWindow... */
	    ptr->next->XWindow = XWindow;
	    return TRUE;
	}
    
    /* We don't have this, add... */    
    tmp = (GlWindowWatcher *) xalloc(sizeof(GlWindowWatcher));
    if (!tmp) return FALSE;
    tmp->XWindow = XWindow;
    tmp->glWindow = glWindow;
    tmp->sock = srvTmp->next->sock;
    tmp->next = list->next;
    list->next = tmp;
    fp = fopen("/home/joshua/Desktop/xvnc_output.txt", "a");
    fprintf(stderr, "Watching window %u (XID %u) for GlStub address %s:%hu, socket %d\n", 
	    glWindow, XWindow, IP2String(address), (unsigned short) port, tmp->sock);
    fprintf(fp, "Watching window %u (XID %u) for GlStub address %s:%hu, socket %d\n", 
	    glWindow, XWindow, IP2String(address), (unsigned short) port, tmp->sock);
    fclose(fp);
    if (thisWindow = DepthSearch(XWindow)) {
	VMGLClipNotify(thisWindow, 0, 0); 
        fp = fopen("/home/joshua/Desktop/xvnc_output.txt", "a");
	fprintf(stderr, "ClipNotify sent for XID %u, glWindow %u\n", XWindow, glWindow);
	fprintf(fp, "ClipNotify sent for XID %u, glWindow %u\n", XWindow, glWindow);
        fclose(fp);
    } else {
	fp = fopen("/home/joshua/Desktop/xvnc_output.txt", "a");
	fprintf(stderr, "Odd, no window found for XID %u\n", XWindow);
	fprintf(fp, "Odd, no window found for XID %u\n", XWindow);
	fclose(fp);
    }
    return TRUE;
}

/* These are X proto wrappers for the extension functions */
static int VMGLInitSrv(ClientPtr client)
{
    REQUEST(xVMGLInitReq);
    XVMGLInitReply reply;

    REQUEST_SIZE_MATCH(xVMGLInitReq);
    reply.retVal = AddGlStub(stuff->address, stuff->port) ? 1:0;

    reply.type = X_Reply;
    reply.length = 0;
    reply.sequenceNumber = client->sequence;
    if(client->swapped)
    {
	register char swapper;
    	swaps(&reply.sequenceNumber, swapper);
    }

    WriteToClient(client, SIZEOF(XVMGLInitReply), (char *) &reply);
    return (client->noClientException);
}

static int VMGLWatchWindowSrv(ClientPtr client)
{
    REQUEST(xVMGLWatchWindowReq);
    XVMGLWatchWindowReply reply;

    REQUEST_SIZE_MATCH(xVMGLWatchWindowReq);
    reply.retVal = AddWindowWatch(stuff->XWindow, stuff->glWindow, 
				  stuff->address, stuff->port) ? 1:0;
    
    reply.type = X_Reply;
    reply.length = 0;
    reply.sequenceNumber = client->sequence;
    if(client->swapped)
    {
	register char swapper;
    	swaps(&reply.sequenceNumber, swapper);
    }

    WriteToClient(client, SIZEOF(XVMGLWatchWindowReply), (char *) &reply);
    return (client->noClientException);
}

static int VMGLResetSrv(ClientPtr client)
{
    REQUEST(xVMGLInitReq);
    XVMGLInitReply reply;

    REQUEST_SIZE_MATCH(xVMGLInitReq);
    reply.retVal = RemoveGlStub(stuff->address, stuff->port) ? 1:0;

    reply.type = X_Reply;
    reply.length = 0;
    reply.sequenceNumber = client->sequence;
    if(client->swapped)
    {
	register char swapper;
    	swaps(&reply.sequenceNumber, swapper);
    }

    WriteToClient(client, SIZEOF(XVMGLInitReply), (char *) &reply);
    return (client->noClientException);
}

/* Dispatching. Boring. Need to handle swapping/endiannes as well */
static int NormalDispatcher(ClientPtr client)
{
    REQUEST(xReq);

    switch (stuff->data)
    {
	case X_VMGLInit:
	    return VMGLInitSrv(client);
	case X_VMGLWatchWindow:
	    return VMGLWatchWindowSrv(client);
	case X_VMGLReset:
	    return VMGLResetSrv(client);
	default:
	    return BadRequest;
    }
}

static int SwappedVMGLInitSrv(ClientPtr client)
{
    REQUEST(xVMGLInitReq);
    register char swapper;

    swaps(&stuff->length, swapper);
    REQUEST_SIZE_MATCH(xVMGLInitReq);
    swaps(&stuff->address, swapper);
    swaps(&stuff->port, swapper);
    return VMGLInitSrv(client);
}

static int SwappedVMGLWatchWindowSrv(ClientPtr client)
{
    REQUEST(xVMGLWatchWindowReq);
    register char swapper;

    swaps(&stuff->length, swapper);
    REQUEST_SIZE_MATCH(xVMGLWatchWindowReq);
    swaps(&stuff->XWindow, swapper);
    swaps(&stuff->glWindow, swapper);
    swaps(&stuff->address, swapper);
    swaps(&stuff->port, swapper);
    return VMGLWatchWindowSrv(client);
}

static int SwappedVMGLResetSrv(ClientPtr client)
{
    REQUEST(xVMGLInitReq);
    register char swapper;

    swaps(&stuff->length, swapper);
    REQUEST_SIZE_MATCH(xVMGLInitReq);
    swaps(&stuff->address, swapper);
    swaps(&stuff->port, swapper);
    return VMGLInitSrv(client);
}

static int SwappedDispatcher(ClientPtr client)
{
    REQUEST(xReq);

    switch (stuff->data)
    {
	case X_VMGLInit:
	    return SwappedVMGLInitSrv(client);
	case X_VMGLWatchWindow:
	    return SwappedVMGLWatchWindowSrv(client);
	case X_VMGLReset:
	    return SwappedVMGLResetSrv(client);
	default:
	    return BadRequest;
    }
}

/* Extension initialization */
static void
VMGLDummyReset(ExtensionEntry *extEntry)
{
}

void VMGLExtensionInit(void)
{
    ExtensionEntry *extEntry;
    int i;

    /* Initialize globals */
    GlStubWatchersList = (GlStubWatcher *) xalloc(sizeof(GlStubWatcher));
    if (!GlStubWatchersList) {
	fp = fopen("/home/joshua/Desktop/xvnc_output.txt", "a");
	fprintf(stderr, "Unable to start VMGL extension\n");
	fprintf(fp, "Unable to start VMGL extension\n");
	fclose(fp);
	return;
    }
    GlStubWatchersList->next = NULL;


    GlWindowWatchersList = (GlWindowWatcher *) xalloc(sizeof(GlWindowWatcher));
    if (!GlWindowWatchersList) {
	fp = fopen("/home/joshua/Desktop/xvnc_output.txt", "a");
	fprintf(stderr, "Unable to start VMGL extension\n");
	fprintf(fp, "Unable to start VMGL extension\n");
	fclose(fp);
	return;
    }
    GlWindowWatchersList->next = NULL;

    /* Initialize extension */
    extEntry = AddExtension(VMGL_EXTENSION_NAME, 0, 0,
			    NormalDispatcher, SwappedDispatcher,
                            VMGLDummyReset, StandardMinorOpcode);
    if (!extEntry) {
	xfree(GlStubWatchersList);
	fp = fopen("/home/joshua/Desktop/xvnc_output.txt", "a");
	fprintf(stderr, "Unable to start VMGL extension\n");
        fprintf(fp, "Unable to start VMGL extension\n");
	fclose(fp);
	return;
    }
    
    for (i = 0; i < screenInfo.numScreens; i++)
    {
        ClipNotify_wrap[i] = screenInfo.screens[i]->ClipNotify;
        screenInfo.screens[i]->ClipNotify = VMGLClipNotify;
        DestroyWindow_wrap[i] = screenInfo.screens[i]->DestroyWindow;
        screenInfo.screens[i]->DestroyWindow = VMGLDestroyWindow;
    }
                                                                                
    return;
}
