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

#include <stdio.h>
#include <X11/Xlibint.h>
#include "Xext.h"
#include "extutil.h"
#include "vmgl_ext.h"
#include "cr_XExtension.h"

/* All the X Extensions Setup bureaucracy */

static char *VMGLExtensionName = VMGL_EXTENSION_NAME;
static in_addr_t glStubIP = 0;
static in_port_t glStubPort = 0;

/* static int close_display(); */
/* static char *error_string(); */
static XExtensionHooks VMGLExtensionHooks = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
/*    close_display, */
    NULL,
    NULL,
    NULL,
    NULL
/*    error_string */
};

/* XXX: Done this with no real clue */
/* static char *VMGL_errl[] = {
     "Dunno",
     "Dunno"
}; */

static XExtensionInfo VMGLXExtensionInfo;
static XExtensionInfo *VMGLXExtensionInfoPtr = &VMGLXExtensionInfo; /* A: BLAHH!!! */

static XEXT_GENERATE_FIND_DISPLAY (VMGLAutogenFindDisplay, 
				   VMGLXExtensionInfoPtr,
				   VMGLExtensionName, 
				   &VMGLExtensionHooks, 
				   0, NULL)

/* static XEXT_GENERATE_CLOSE_DISPLAY (close_display, VMGLXExtensionInfoPtr) */

/* static XEXT_GENERATE_ERROR_STRING(error_string, VMGLExtensionName,
			   0, VMGL_errl) */
			   /* XXX for arg 0 */

/* Extension calls */
Status XVMGLExtInit (Display *dpy, in_addr_t address, in_port_t port)
{
    XExtDisplayInfo *ExtensionInfo = VMGLAutogenFindDisplay(dpy);
    XVMGLInitReply XInternalReply;
    register xVMGLInitReq *XProtoRequest;

    if (!XextHasExtension (ExtensionInfo))
        return ((Status) 0);

    LockDisplay (dpy);

    GetReq(VMGLInit, XProtoRequest);
    XProtoRequest->reqType = ExtensionInfo->codes->major_opcode;
    XProtoRequest->VMGLRequestType = X_VMGLInit;
    XProtoRequest->address = address;
    XProtoRequest->port = port;

    if (!_XReply (dpy, (xReply *) &XInternalReply, 0, xTrue)) {
	UnlockDisplay (dpy);
	SyncHandle ();
	return (Status)0;
    }

    UnlockDisplay (dpy);
    SyncHandle ();
    if (XInternalReply.retVal) {
	glStubIP = address;
	glStubPort = port;
	return ((Status) 1);
    } else
	return ((Status) 0);
}

Status XVMGLExtWatchWindow(Display *dpy, unsigned int glWindow, XID XWindow )
{
    XExtDisplayInfo *ExtensionInfo = VMGLAutogenFindDisplay(dpy);
    XVMGLWatchWindowReply XInternalReply;
    register xVMGLWatchWindowReq *XProtoRequest;
    
    /* Goofy sanity check */
    if (!(glStubIP && glStubPort))
	return ((Status) 0);

    if (!XextHasExtension (ExtensionInfo))
        return ((Status) 0);

    LockDisplay (dpy);

    GetReq(VMGLWatchWindow, XProtoRequest);
    XProtoRequest->reqType = ExtensionInfo->codes->major_opcode;
    XProtoRequest->VMGLRequestType = X_VMGLWatchWindow;
    XProtoRequest->glWindow = glWindow;
    XProtoRequest->XWindow = XWindow;
    XProtoRequest->address = glStubIP;
    XProtoRequest->port = glStubPort;

    if (!_XReply (dpy, (xReply *) &XInternalReply, 0, xTrue)) {
	UnlockDisplay (dpy);
	SyncHandle ();
	return (Status)0;
    }
    
    UnlockDisplay (dpy);
    SyncHandle ();
    return ((Status) XInternalReply.retVal);
}

/* This uses the same message pair as XVMGLInit, because I'm lazy */
Status XVMGLExtReset (Display *dpy)
{
    XExtDisplayInfo *ExtensionInfo = VMGLAutogenFindDisplay(dpy);
    XVMGLInitReply XInternalReply;
    register xVMGLInitReq *XProtoRequest;

    if (!XextHasExtension (ExtensionInfo))
        return ((Status) 0);

    LockDisplay (dpy);

    GetReq(VMGLInit, XProtoRequest);
    XProtoRequest->reqType = ExtensionInfo->codes->major_opcode;
    XProtoRequest->VMGLRequestType = X_VMGLReset;
    XProtoRequest->address = glStubIP;
    XProtoRequest->port = glStubPort;

    if (!_XReply (dpy, (xReply *) &XInternalReply, 0, xTrue)) {
	UnlockDisplay (dpy);
	SyncHandle ();
	return (Status)0;
    }

    UnlockDisplay (dpy);
    SyncHandle ();
    if (XInternalReply.retVal) {
	glStubIP = 0;
	glStubPort = 0;
	return ((Status) 1);
    } else
	return ((Status) 0);
}

