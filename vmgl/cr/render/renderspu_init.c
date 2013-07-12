/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_mem.h"
#include "cr_spu.h"
#include "cr_error.h"
#include "cr_string.h"
#include "cr_url.h"
#include "renderspu.h"
#include <stdio.h>

static SPUNamedFunctionTable _cr_render_table[1000];

SPUFunctions render_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	_cr_render_table /* THE ACTUAL FUNCTIONS */
};

RenderSPU render_spu;

#ifdef CHROMIUM_THREADSAFE
CRtsd _RenderTSD;
#endif

static void swapsyncConnect(void)
{
	char hostname[4096], protocol[4096];
	unsigned short port;

	crNetInit(NULL, NULL);

	if (!crParseURL( render_spu.swap_master_url, protocol, hostname, 
					&port, 9876))
		crError( "Bad URL: %s", render_spu.swap_master_url );

	if (render_spu.is_swap_master)
	{
		int a;

		render_spu.swap_conns = (CRConnection **)crAlloc(
						render_spu.num_swap_clients*sizeof(CRConnection *));
		for (a=0; a<render_spu.num_swap_clients; a++)
		{
			render_spu.swap_conns[a] = crNetAcceptClient( protocol, hostname, port,render_spu.swap_mtu);
						    /* This used to be brokered, unknown. Andres */
		}
	}
	else
	{
		render_spu.swap_conns = (CRConnection **)crAlloc(sizeof(CRConnection *));

		render_spu.swap_conns[0] = crNetConnectToServer(render_spu.swap_master_url,port, render_spu.swap_mtu );
					    /* This used to be brokered, unknown. Andres */
		if (!render_spu.swap_conns[0])
			crError("Failed connection");
	}
}


static SPUFunctions *
renderSPUInit( int id, SPU *child, SPU *self,
               unsigned int context_id, unsigned int num_contexts )
{
	int numFuncs, numSpecial;
	GLint defaultWin, defaultCtx;
	WindowInfo *windowInfo;

	(void) child;
	(void) context_id;
	(void) num_contexts;

	self->privatePtr = (void *) &render_spu;

#ifdef CHROMIUM_THREADSAFE
	crDebug("Render SPU: thread-safe");
	crInitTSD(&_RenderTSD);
#endif

	crMemZero(&render_spu, sizeof(render_spu));

	render_spu.id = id;
	renderspuGatherConfiguration(&render_spu);
	if (render_spu.swap_master_url)
		swapsyncConnect();


	/* Get our special functions. */
	numSpecial = renderspuCreateFunctions( _cr_render_table );

	/* Get the OpenGL functions. */
	numFuncs = crLoadOpenGL( &render_spu.ws, _cr_render_table + numSpecial );
	if (numFuncs == 0) {
		crError("The render SPU was unable to load the native OpenGL library");
		return NULL;
	}

	numFuncs += numSpecial;

#ifdef GLX
	if (!render_spu.use_glxchoosevisual) {
		/* sometimes want to set this option with ATI drivers */
		render_spu.ws.glXChooseVisual = NULL;
	}
#endif

	render_spu.window_id = 0;
	render_spu.context_id = 0;
	render_spu.contextTable = crAllocHashtable();
	render_spu.windowTable = crAllocHashtable();

	CRASSERT(render_spu.default_visual & CR_RGB_BIT);

#ifdef USE_OSMESA
	if (render_spu.use_osmesa) {
		if (!crLoadOSMesa(&render_spu.OSMesaCreateContext,
						  &render_spu.OSMesaMakeCurrent,
						  &render_spu.OSMesaDestroyContext)) {
			crError("Unable to load OSMesa library");
		}
	}
#endif

	/*
	 * Create the default window and context.  Their indexes are zero and
	 * a client can use them without calling CreateContext or WindowCreate.
	 */
	crDebug("Render SPU: Creating default window (visBits=0x%x, id=0)",
					render_spu.default_visual);
	defaultWin = renderspuWindowCreate( NULL, render_spu.default_visual );
	if (defaultWin != 0) {
		crError("Render SPU: Couldn't get a double-buffered, RGB visual with Z!");
	}
	crDebug( "Render SPU: WindowCreate returned %d (0=normal)", defaultWin );

	crDebug("Render SPU: Creating default context, visBits=0x%x",
					render_spu.default_visual );
	defaultCtx = renderspuCreateContext( NULL, render_spu.default_visual, 0 );
	CRASSERT(defaultCtx == 0);

	renderspuMakeCurrent( defaultWin, 0, defaultCtx );

	/* Get windowInfo for the default window */
	windowInfo = (WindowInfo *) crHashtableSearch(render_spu.windowTable, 0);
	CRASSERT(windowInfo);
	windowInfo->mapPending = GL_TRUE;

	/*
	 * Get the OpenGL extension functions.
	 * SIGH -- we have to wait until the very bitter end to load the 
	 * extensions, because the context has to be bound before
	 * wglGetProcAddress will work correctly.  No such issue with GLX though.
	 */
	numFuncs += crLoadOpenGLExtensions( &render_spu.ws, _cr_render_table + numFuncs );
	CRASSERT(numFuncs < 1000);

	render_spu.barrierHash = crAllocHashtable();

	render_spu.cursorX = 0;
	render_spu.cursorY = 0;
	render_spu.use_L2 = 0;

	render_spu.gather_conns = NULL;

	crDebug("Render SPU: ---------- End of Init -------------");
	return &render_functions;
}


static void renderSPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(render_spu.self) );
	crSPUCopyDispatchTable( &(render_spu.self), self );

	render_spu.server = (GLStub *)(self->server);
}


static void DeleteContextCallback( void *data )
{
	ContextInfo *context = (ContextInfo *) data;
	renderspu_SystemDestroyContext(context);
	crFree(context);
}

static void DeleteWindowCallback( void *data )
{
	WindowInfo *window = (WindowInfo *) data;
	renderspu_SystemDestroyWindow(window);
	crFree(window);
}

static int renderSPUCleanup(void)
{
	crFreeHashtable(render_spu.contextTable, DeleteContextCallback);
	render_spu.contextTable = NULL;
	crFreeHashtable(render_spu.windowTable, DeleteWindowCallback);
	render_spu.windowTable = NULL;
	crFreeHashtable(render_spu.barrierHash, crFree);
	render_spu.barrierHash = NULL;
	return 1;
}


extern SPUOptions renderSPUOptions[];

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options, int *flags )
{
	*name = "render";
	*super = NULL;
	*init = renderSPUInit;
	*self = renderSPUSelfDispatch;
	*cleanup = renderSPUCleanup;
	*options = renderSPUOptions;
	*flags = (SPU_NO_PACKER|SPU_IS_TERMINAL|SPU_MAX_SERVERS_ZERO);
	
	return 1;
}
