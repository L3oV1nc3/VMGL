/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "packspu.h"
#include "cr_mem.h"
#include "cr_packfunctions.h"
#include "cr_string.h"
#include "packspu_proto.h"
#include "cr_XExtension.h"

//#define MAGIC_OFFSET 3000

/*
 * Allocate a new ThreadInfo structure, setup a connection to the
 * server, allocate/init a packer context, bind this ThreadInfo to
 * the calling thread with crSetTSD().
 * We'll always call this function at least once even if we're not
 * using threads.
 */
ThreadInfo *packspuNewThread( CRthread id )
{
	ThreadInfo *thread;

#ifdef CHROMIUM_THREADSAFE
	crLockMutex(&_PackMutex);
#else
	CRASSERT(pack_spu.numThreads == 0);
#endif

	CRASSERT(pack_spu.numThreads < MAX_THREADS);
	thread = &(pack_spu.thread[pack_spu.numThreads]);

	thread->id = id;
	thread->currentContext = NULL;

	/* connect to the server */
	thread->netServer.name = crStrdup( pack_spu.name );
	thread->netServer.buffer_size = pack_spu.buffer_size;
	if (pack_spu.numThreads == 0) {
		packspuConnectToServer( &(thread->netServer) );
		CRASSERT(thread->netServer.conn);
		pack_spu.swap = thread->netServer.conn->swap;
	}
	else {
		/* a new pthread */
		crNetNewClient( pack_spu.thread[0].netServer.conn, &(thread->netServer));
		CRASSERT(thread->netServer.conn);
	}

	/* packer setup */
	CRASSERT(thread->packer == NULL);
	thread->packer = crPackNewContext( pack_spu.swap );
	CRASSERT(thread->packer);
	crPackInitBuffer( &(thread->buffer), crNetAlloc(thread->netServer.conn),
				thread->netServer.conn->buffer_size, thread->netServer.conn->mtu );
	thread->buffer.canBarf = thread->netServer.conn->Barf ? GL_TRUE : GL_FALSE;
	crPackSetBuffer( thread->packer, &thread->buffer );
	crPackFlushFunc( thread->packer, packspuFlush );
	crPackFlushArg( thread->packer, (void *) thread );
	crPackSendHugeFunc( thread->packer, packspuHuge );
	crPackSetContext( thread->packer );

#ifdef CHROMIUM_THREADSAFE
	crSetTSD(&_PackTSD, thread);
#endif

	pack_spu.numThreads++;

#ifdef CHROMIUM_THREADSAFE
	crUnlockMutex(&_PackMutex);
#endif
	return thread;
}

void openXDisplay(const char *dpyName)
{
    /* Open the named display */
    if (!pack_spu.XDisplay) {
	pack_spu.XDisplay = XOpenDisplay(dpyName);
        if (!pack_spu.XDisplay) {
            pack_spu.XDisplay = XOpenDisplay(":0");
    	    if (!pack_spu.XDisplay) {
        	crInfo("Pack SPU: Can't open X Display");
        	return;
    	    }
        }
    }

    CRASSERT(pack_spu.XDisplay);
    if (!XVMGLExtInit(pack_spu.XDisplay, pack_spu.serverIP, pack_spu.secondPort)) {
	crInfo("Pack SPU: Couldn't init X server CR support extension!!");
	XCloseDisplay(pack_spu.XDisplay);
	pack_spu.XDisplay = (Display *) NULL;
	return;
    }
    pack_spu.openedXDisplay = XDPY_CONNECTED;
    crDebug("Pack SPU: Opened X Display %s",dpyName);
}

GLint PACKSPU_APIENTRY
packspu_CreateContext( const char *dpyName, GLint visual, GLint shareCtx )
{
	int writeback = 1;
	GLint serverCtx = (GLint) -1;
	int slot;
	ContextInfo *contextPtr, *sharedContextPtr = NULL;
	GLint sharedServerCtx = (GLint) -1;

#ifdef CHROMIUM_THREADSAFE
	crLockMutex(&_PackMutex);
#endif
	if (pack_spu.openedXDisplay == XDPY_NEED_CONNECT)
	    openXDisplay(dpyName);

	if (shareCtx > 0) {
		/* Translate to server ctx id */
		shareCtx -= MAGIC_OFFSET;
		if (shareCtx >= 0 && shareCtx < pack_spu.numContexts) {
			sharedContextPtr = &(pack_spu.context[shareCtx]);
			sharedServerCtx = sharedContextPtr->serverCtx;
		}
		shareCtx += MAGIC_OFFSET;
	}

	crPackSetContext( pack_spu.thread[0].packer );

	/* Pack the command */
	if (pack_spu.swap)
		crPackCreateContextSWAP( dpyName, visual, sharedServerCtx, &serverCtx, &writeback );
	else
		crPackCreateContext( dpyName, visual, sharedServerCtx, &serverCtx, &writeback );

	/* Flush buffer and get return value */
	packspuFlush( &(pack_spu.thread[0]) );
	if (!(pack_spu.thread[0].netServer.conn->actual_network))
	{
		/* HUMUNGOUS HACK TO MATCH SERVER NUMBERING
		 *
		 * The hack exists solely to make file networking work for now.  This
		 * is totally gross, but since the server expects the numbers to start
		 * from 5000, we need to write them out this way.  This would be
		 * marginally less gross if the numbers (500 and 5000) were maybe
		 * some sort of #define'd constants somewhere so the client and the
		 * server could be aware of how each other were numbering things in
		 * cases like file networking where they actually
		 * care. 
		 *
		 * 	-Humper 
		 *
		 */
		serverCtx = 5000;
	}
	else {
		while (writeback)
			crNetRecv();

		if (pack_spu.swap) {
			serverCtx = (GLint) SWAP32(serverCtx);
		}
		if (serverCtx < 0) {
#ifdef CHROMIUM_THREADSAFE
			crUnlockMutex(&_PackMutex);
#endif
			crWarning("Failure in packspu_CreateContext");
			return -1;  /* failed */
		}
	}

	/* find an empty context slot */
	for (slot = 0; slot < pack_spu.numContexts; slot++) {
		if (!pack_spu.context[slot].clientState) {
			/* found empty slot */
			break;
		}
	}
	if (slot == pack_spu.numContexts) {
		pack_spu.numContexts++;
	}

	/* Fill in the new context info */
	/* XXX fix-up sharedCtx param here */
	contextPtr = &(pack_spu.context[slot]);
	if (sharedContextPtr) {
	    contextPtr->DLM = sharedContextPtr->DLM;
	    crDLMUseDLM(contextPtr->DLM);
	    contextPtr->clientState = crStateCreateContext(NULL, visual, sharedContextPtr->clientState);
	} else {
	    contextPtr->DLM = crDLMNewDLM(0, NULL);
	    if (!(contextPtr->DLM))
		crWarning("Failed to allocate a DLM for this context.");
	    contextPtr->clientState = crStateCreateContext(NULL, visual, NULL);
	}
	contextPtr->serverCtx = serverCtx;
	contextPtr->visual = visual;
	contextPtr->boundWindow = 0;	
	contextPtr->shareCtx = shareCtx;
	contextPtr->DL_mode = GL_FALSE;
	contextPtr->DL_ID = 0;
	contextPtr->DLMState = crDLMNewContext(contextPtr->DLM);
	
	/* Not any more in our sequence of contexts */
	crListPushBack(pack_spu.contextList, (void *) slot);

#ifdef CHROMIUM_THREADSAFE
	crUnlockMutex(&_PackMutex);
#endif

	return MAGIC_OFFSET + slot;
}

static int integerCompareFunc(const void *e1, const void *e2) {
    return ( ((int) e1) == ((int) e2) ) ? 0 : 1;
}

void PACKSPU_APIENTRY packspu_DestroyContext( GLint ctx )
{
	const int slot = ctx - MAGIC_OFFSET;
	ContextInfo *context;
	GET_THREAD(thread);

	CRASSERT(slot >= 0);
	CRASSERT(slot < pack_spu.numContexts);
	CRASSERT(thread);

	context = &(pack_spu.context[slot]);

	if (pack_spu.swap)
		crPackDestroyContextSWAP( context->serverCtx );
	else
		crPackDestroyContext( context->serverCtx );

	crStateDestroyContext( context->clientState );

	context->clientState = NULL;
	context->serverCtx = 0;
	
	crDLMFreeDLM(context->DLM);
	crDLMFreeContext(context->DLMState);

	if (thread->currentContext == context) {
		thread->currentContext = NULL;
		crStateMakeCurrent( NULL );
		crDLMSetCurrentState( NULL );
	}
	
	/* Now delete from list as well */
	CRListIterator *thisContext = crListFind(pack_spu.contextList, (void *) slot, integerCompareFunc);
	if (thisContext)
	    crListErase(pack_spu.contextList, thisContext);
}


void PACKSPU_APIENTRY packspu_MakeCurrent( GLint window, GLint nativeWindow, GLint ctx )
{
	GET_THREAD(thread);
	GLint serverCtx;
	ContextInfo *newCtx;
	WindowInfo *WInfo = (WindowInfo *) crHashtableSearch(pack_spu.XWindows,window);
	/* int showMe = 0; 
	XWindowAttributes windowAttributes; */

	if (!thread) {
		thread = packspuNewThread( crThreadID() );
	}
	CRASSERT(thread);
	CRASSERT(thread->packer);

	if (ctx) {
		const int slot = ctx - MAGIC_OFFSET;

		CRASSERT(slot >= 0);
		CRASSERT(slot < pack_spu.numContexts);

		newCtx = &pack_spu.context[slot];
		CRASSERT(newCtx->clientState);  /* verify valid */

		thread->currentContext = newCtx;
		
		if (WInfo && (pack_spu.openedXDisplay == XDPY_CONNECTED) && (WInfo->XWindow != nativeWindow)) {
		    WInfo->XWindow = nativeWindow;
		    if (XVMGLExtWatchWindow(pack_spu.XDisplay, window, WInfo->XWindow)) {
			crDebug("X server watching window %d XID %d", window, nativeWindow);
        		/* Because the XServer will start watching this window 
        		   as of now, make sure it "shows" */
/*			XSync(pack_spu.XDisplay,0);
    			XGetWindowAttributes(pack_spu.XDisplay, (Window) WInfo->XWindow, 
    					     &windowAttributes);
			showMe = (windowAttributes.map_state == IsViewable); */
        		/* XSync(pack_spu.XDisplay,0);  */
        	    } else 
        		crInfo("XVMGLExtWatchWindow for window %d failed", window);
		}

		crPackSetContext( thread->packer );
		crStateMakeCurrent( newCtx->clientState );
		crDLMSetCurrentState( newCtx->DLMState);
		serverCtx = pack_spu.context[slot].serverCtx;
		newCtx->boundWindow = window;
	}
	else {
		thread->currentContext = NULL;
		crStateMakeCurrent( NULL );
		crDLMSetCurrentState( NULL );
		newCtx = NULL;
		serverCtx = 0;
	}

	if (pack_spu.swap)
		crPackMakeCurrentSWAP( window, nativeWindow, serverCtx );
	else
		crPackMakeCurrent( window, nativeWindow, serverCtx );
/*	if (showMe) {
	    if (pack_spu.swap)
		crPackWindowShowSWAP( window, GL_TRUE );
	    else
		crPackWindowShow( window, GL_TRUE );
	} */

	{
		GET_THREAD(t);
		(void) t;
		CRASSERT(t);
	}
}
