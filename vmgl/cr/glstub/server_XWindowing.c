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

/* vmgl project */
#if defined (SunOS)
#include <X11/Xlib.h>
#endif
#include <X11/extensions/shape.h>
#include <unistd.h>
#include <string.h>

#include "cr_mem.h"
#include "cr_environment.h"
#include "cr_string.h"
#include "cr_error.h"
#include "cr_glstate.h"
#include "server.h"
#include "vmgl_extproto.h"

/* Reduce annoyance. Copy textual definition of BoxRec, BoxPtr. 
   Could apparently borrow it from Xregion.h */
typedef struct _Box {
    short x1, y1, x2, y2;
} BoxRec;
typedef struct _Box *BoxPtr;

static void messageHandler(int sock) {
    XWindowData winData;
    CRMuralInfo *Mural;
    XVMGLWindowingCommand msg;
    BoxPtr clipBoxes;
    XRectangle *clipRects;
    size_t msgSize = sizeof(msg);
    int readBytes, x, y;
    unsigned int i, numRects, len, width, height;
    unsigned long visibleArea = 0;

    /* First read the msg header */
    if ((readBytes = (int) read(sock, (void *) &msg, msgSize)) != (int) msgSize)
        crError("XWindowing thread error -> read from socket %d", readBytes);

    /* Find the XID/dpy of the window */
    Mural = (CRMuralInfo *) crHashtableSearch(cr_server.muralTable, ntohl(msg.glWindow));
    if (Mural)
	cr_server.head_spu->dispatch_table.GetChromiumParametervCR(
			    GL_WINDOW_XID_CR, Mural->spuWindow, GL_INT, 1, &winData);
    else winData.XWindow = 0;

    /* Make sure we read everything sent */
    numRects = ntohl(msg.length);
    len = numRects * sizeof(BoxRec);
    clipBoxes = (BoxPtr) crAlloc((size_t) len);
    if (read(sock, (void *) clipBoxes, len) != ((ssize_t) len))
    	crError("XWindowing thread error -> read boxes from socket");

    /* Discard msg if bad window ID */
    if (!winData.XWindow) {
        crFree(clipBoxes);
        crInfo("XWindowing thread error -> unknown window ID read");
        return;
    }

    /* Get remaining msg fields */
    x = (int) ntohl(msg.x);
    y = (int) ntohl(msg.y);
    width = ntohl(msg.width);
    height = ntohl(msg.height);

    /* Well formed clipping request, we have all the data */
    clipRects = crAlloc(sizeof(XRectangle)*numRects);
    for (i=0;i<numRects;i++) {
	/* XRectangle: x,y,width,height */
	/* BoxRec: x1,y1,x2,y2*/
	clipRects[i].x = clipBoxes[i].x1 - x;
	clipRects[i].y = clipBoxes[i].y1 - y;
	clipRects[i].width = clipBoxes[i].x2 - clipBoxes[i].x1;
	clipRects[i].height = clipBoxes[i].y2 - clipBoxes[i].y1;
	visibleArea += ((unsigned long) clipRects[i].width) * ((unsigned long) clipRects[i].height);
	crDebug("Rect %d %u %u %u %u", i, clipRects[i].x, clipRects[i].y, clipRects[i].width, clipRects[i].height);
    }
	
    /* Go for it */
    XLockDisplay(winData.dpy);
/*    XMoveWindow(winData.dpy, winData.XWindow, x, y);
    XResizeWindow(winData.dpy, winData.XWindow, width, height); */
    XMoveResizeWindow(winData.dpy, winData.XWindow, x, y, width, height);
    Mural->visibleArea = visibleArea;
    /* Might consider unmapping if numRects is zero.
       However, need to remember that it's unmapped, and remap it
       when new request with numRects > zero arrives */
    XShapeCombineRectangles(winData.dpy, winData.XWindow, ShapeBounding, 0, 0,
				clipRects, numRects, ShapeSet, 0);
    XSync(winData.dpy, 0);
    XUnlockDisplay(winData.dpy);
	
    crFree(clipBoxes);
    crFree(clipRects);
		    
}

static void *XWindowingHandler(void *blah) {
    int sock, srvSock;
    unsigned int tmp;
    struct sockaddr_in addr;
    size_t lenAddr = sizeof (struct sockaddr_in);
    unsigned short port = cr_server.secondPort;
    
    if ((srvSock = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
	crError("XWindowing thread error -> create socket");
    memset((void *) &addr, 0, lenAddr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(srvSock, (struct sockaddr *) &addr, lenAddr))
	crError("XWindowing thread error -> bind socket to port %hu", port);
    if (listen(srvSock,1))
	crError("XWindowing thread error -> listen on socket");
    crDebug("XWindowing thread set, listening on port %hu", port);
    if ((sock = accept(srvSock, (struct sockaddr *) NULL, &tmp)) <= 0)
	crError("accept on socket");
    crDebug("XWindowing thread accepted connection");

    for(;;) {
	messageHandler(sock);
    }

    /* Should never arrive here */
    if (close(sock)||close(srvSock))
	crInfo("Xwindowing thread error -> close on socket");
    return blah;
}

void spawnXWindowingThread() {
    pthread_t thread;
    
    pthread_create(&thread,NULL,XWindowingHandler,NULL);
    return;    
}
