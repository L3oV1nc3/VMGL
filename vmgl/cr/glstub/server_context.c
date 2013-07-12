/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */
	
#include "cr_spu.h"
#include "chromium.h"
#include "cr_error.h"
#include "cr_net.h"
#include "cr_rand.h"
#include "server_dispatch.h"
#include "server.h"

/**
 * Generate a new window ID.
 */
static GLint
generateID(void)
{
	if (cr_server.uniqueWindows) {
		static GLboolean firstCall = GL_TRUE;
		if (firstCall) {
			crRandAutoSeed();
			firstCall = GL_FALSE;
		}
		return crRandInt(20000, 50000); /* XXX FIX */
	}
	else {
		static GLint freeID = 1;
		return freeID++;
	}
}


GLint SERVER_DISPATCH_APIENTRY
glStubDispatchCreateContext( const char *dpyName, GLint visualBits, GLint shareCtx )
{
	GLint retVal = -1;
	CRContext *newCtx;

	if (shareCtx > 0) {
		crWarning("GLStub: context sharing not implemented.");
		shareCtx = 0;
	}

	/* Since the Cr server serialized all incoming clients/contexts into
	 * one outgoing GL stream, we only need to create one context for the
	 * head SPU.  We'll only have to make it current once too, below.
	 */
	if (cr_server.firstCallCreateContext) {
		cr_server.SpuContextVisBits = visualBits;
		cr_server.SpuContext = cr_server.head_spu->dispatch_table.
			CreateContext(dpyName, cr_server.SpuContextVisBits, shareCtx);
		if (cr_server.SpuContext < 0) {
			crWarning("glStubDispatchCreateContext() failed.");
			return -1;
		}
		cr_server.firstCallCreateContext = GL_FALSE;
	}
	else {
		/* second or third or ... context */
		if ((visualBits & cr_server.SpuContextVisBits) != visualBits) {
			int oldSpuContext;

			/* the new context needs new visual attributes */
			cr_server.SpuContextVisBits |= visualBits;
			crDebug("glStubDispatchCreateContext requires new visual (0x%x).",
							cr_server.SpuContextVisBits);

			/* Here, we used to just destroy the old rendering context.
			 * Unfortunately, this had the side effect of destroying
			 * all display lists and textures that had been loaded on
			 * the old context as well.
			 *
			 * Now, first try to create a new context, with a suitable
			 * visual, sharing display lists and textures with the
			 * old context.  Then destroy the old context.
			 */

			/* create new rendering context with suitable visual */
			oldSpuContext = cr_server.SpuContext;
			cr_server.SpuContext = cr_server.head_spu->dispatch_table.
				CreateContext(dpyName, cr_server.SpuContextVisBits, cr_server.SpuContext);
			/* destroy old rendering context */
			cr_server.head_spu->dispatch_table.DestroyContext(oldSpuContext);
			if (cr_server.SpuContext < 0) {
				crWarning("glStubDispatchCreateContext() failed.");
				return -1;
			}
		}
	}

	/* Now create a new state-tracker context and initialize the
	 * dispatch function pointers.
	 */
	newCtx = crStateCreateContext( &cr_server.limits, visualBits, NULL );
	if (newCtx) {
		crStateSetCurrentPointers( newCtx, &(cr_server.current) );
		retVal = generateID();
		crHashtableAdd(cr_server.contextTable, retVal, newCtx);
	}

	glStubReturnValue( &retVal, sizeof(retVal) );
	return retVal;
}


void SERVER_DISPATCH_APIENTRY
glStubDispatchDestroyContext( GLint ctx )
{
	CRContext *crCtx;

	crCtx = (CRContext *) crHashtableSearch(cr_server.contextTable, ctx);
	if (!crCtx) {
		crWarning("GLStub: DestroyContext invalid context %d", ctx);
		return;
	}

	crHashtableDelete(cr_server.contextTable, ctx, NULL);
	crStateDestroyContext( crCtx );

	/* If we delete our current context, default back to the null context */
	if (cr_server.curClient->currentCtx == crCtx) {
		cr_server.curClient->currentContextNumber = -1;
		cr_server.curClient->currentCtx = cr_server.DummyContext;
	}
}


void SERVER_DISPATCH_APIENTRY
glStubDispatchMakeCurrent( GLint window, GLint nativeWindow, GLint context )
{
	CRMuralInfo *mural;
	CRContext *ctx;

	if (context >= 0 && window >= 0) {
		mural = (CRMuralInfo *) crHashtableSearch(cr_server.muralTable, window);
		if (!mural && window == MAGIC_OFFSET &&
				!cr_server.clients[0]->conn->actual_network) {
			/* We're reading from a file and not a real network connection so
			 * we have to fudge the window id here.
			 */
			window = 0;
			mural = (CRMuralInfo *) crHashtableSearch(cr_server.muralTable, 0);
		}
		CRASSERT(mural);

		/* Update the state tracker's current context */
		ctx = (CRContext *) crHashtableSearch(cr_server.contextTable, context);
		if (!ctx) {
			crWarning("GLStub: NULL context in MakeCurrent %d", context);
			return;
		}
	}
	else {
		ctx = cr_server.DummyContext;
		window = -1;
		mural = NULL;
		return;
	}

	/*
	crDebug("**** %s client %d  curCtx=%d curWin=%d", __func__,
					cr_server.curClient->number, ctxPos, window);
	*/
	cr_server.curClient->currentContextNumber = context;
	cr_server.curClient->currentCtx = ctx;
	cr_server.curClient->currentMural = mural;
	cr_server.curClient->currentWindow = window;

	CRASSERT(cr_server.curClient->currentCtx);

	/* This is a hack to force updating the 'current' attribs */
	crStateUpdateColorBits();

	if (ctx)
		crStateSetCurrentPointers( ctx, &(cr_server.current) );

	/* check if being made current for first time, update viewport */
	if (ctx) {
		/* initialize the viewport */
		if (ctx->viewport.viewportW == 0) {
			ctx->viewport.viewportW = mural->width;
			ctx->viewport.viewportH = mural->height;
			ctx->viewport.scissorW = mural->width;
			ctx->viewport.scissorH = mural->height;
		}
	}

	/*
	crDebug("**** %s  currentWindow %d  newWindow %d", __func__,
					cr_server.currentWindow, window);
	*/

	if (1/*cr_server.firstCallMakeCurrent ||
			cr_server.currentWindow != window ||
			cr_server.currentNativeWindow != nativeWindow*/) {
		/* Since the cr server serialized all incoming contexts/clients into
		 * one output stream of GL commands, we only need to call the head
		 * SPU's MakeCurrent() function once.
		 * BUT, if we're rendering to multiple windows, we do have to issue
		 * MakeCurrent() calls sometimes.  The same GL context will always be
		 * used though.
		 */
		cr_server.head_spu->dispatch_table.MakeCurrent( mural->spuWindow,
																										nativeWindow,
																										cr_server.SpuContext );
		cr_server.firstCallMakeCurrent = GL_FALSE;
		cr_server.currentWindow = window;
		cr_server.currentNativeWindow = nativeWindow;

		/* Set initial raster/window position for this context.
		 * The position has to be translated according to the tile origin.
		 */
		if (mural->numExtents > 0)
		{
			GLint x = -mural->extents[0].imagewindow.x1;
			GLint y = -mural->extents[0].imagewindow.y1;
			cr_server.head_spu->dispatch_table.WindowPos2iARB(x, y);
			/* This MakeCurrent is a bit redundant (we do it again below)
			 * but it's only done the first time we activate a context.
			 */
			crStateMakeCurrent(ctx);
			crStateWindowPos2iARB(x, y);
		}
	}

	/* This used to be earlier, after crStateUpdateColorBits() call */
	crStateMakeCurrent( ctx );

	/* This is pessimistic - we really don't have to invalidate the viewport
	 * info every time we MakeCurrent, but play it safe for now.
	 */
	mural->viewportValidated = GL_FALSE;
}

