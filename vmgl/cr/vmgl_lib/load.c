/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "cr_net.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_string.h"
#include "cr_net.h"
#include "cr_environment.h"
#include "cr_process.h"
#include "cr_rand.h"
#include "stub.h"
#include <stdlib.h>
#include <signal.h>

/**
 * If you change this, see the comments in tilesortspu_context.c
 */
#define MAGIC_CONTEXT_BASE 500

/* NOTE: 'SPUDispatchTable glim' is declared in NULLfuncs.py now */
/* NOTE: 'SPUDispatchTable stubThreadsafeDispatch' is declared in tsfuncs.c */
Stub stub;


static void stubInitNativeDispatch( void )
{
#define MAX_FUNCS 1000
	SPUNamedFunctionTable gl_funcs[MAX_FUNCS];
	int numFuncs;

	numFuncs = crLoadOpenGL( &stub.wsInterface, gl_funcs );

	stub.haveNativeOpenGL = (numFuncs > 0);

	/* XXX call this after context binding */
	numFuncs += crLoadOpenGLExtensions( &stub.wsInterface, gl_funcs + numFuncs );

	CRASSERT(numFuncs < MAX_FUNCS);

	crSPUInitDispatchTable( &stub.nativeDispatch );
	crSPUInitDispatch( &stub.nativeDispatch, gl_funcs );
	crSPUInitDispatchNops( &stub.nativeDispatch );
#undef MAX_FUNCS
}


/** Pointer to the SPU's real glClear and glViewport functions */
static ClearFunc_t origClear;
static ViewportFunc_t origViewport;


static void stubCheckWindowSize(void)
{
	int winX, winY;
	unsigned int winW, winH;
	WindowInfo *window;

	CRASSERT(stub.trackWindowSize || stub.trackWindowPos);

	if (!stub.currentContext)
		return;

	window = stub.currentContext->currentDrawable;

	stubGetWindowGeometry( window, &winX, &winY, &winW, &winH );

	if (winW && winH) {
		if (stub.trackWindowSize) {
			if (winW != window->width || winH != window->height) {
				if (window->type == CHROMIUM)
					stub.spuDispatch.WindowSize( window->spuWindow, winW, winH );
				window->width = winW;
				window->height = winH;
			}
		}
		if (stub.trackWindowPos) {
			if (winX != window->x || winY != window->y) {
				if (window->type == CHROMIUM)
					stub.spuDispatch.WindowPosition( window->spuWindow, winX, winY );
				window->x = winX;
				window->y = winY;
			}
		}
	}
}


/**
 * Override the head SPU's glClear function.
 * We're basically trapping this function so that we can poll the
 * application window size at a regular interval.
 */
static void SPU_APIENTRY trapClear(GLbitfield mask)
{
	stubCheckWindowSize();
	/* call the original SPU glClear function */
	origClear(mask);
}

/**
 * As above, but for glViewport.  Most apps call glViewport before
 * glClear when a window is resized.
 */
static void SPU_APIENTRY trapViewport(GLint x, GLint y, GLsizei w, GLsizei h)
{
	stubCheckWindowSize();
	/* call the original SPU glViewport function */
	origViewport(x, y, w, h);
}


/**
 * Use the GL function pointers in <spu> to initialize the static glim
 * dispatch table.
 */
static void stubInitSPUDispatch(SPU *spu)
{
	crSPUInitDispatchTable( &stub.spuDispatch );
	crSPUCopyDispatchTable( &stub.spuDispatch, &(spu->dispatch_table) );

	if (stub.trackWindowSize || stub.trackWindowPos) {
		/* patch-in special glClear/Viewport function to track window sizing */
		origClear = stub.spuDispatch.Clear;
		origViewport = stub.spuDispatch.Viewport;
		stub.spuDispatch.Clear = trapClear;
		stub.spuDispatch.Viewport = trapViewport;
	}

	crSPUCopyDispatchTable( &glim, &stub.spuDispatch );
}


/**
 * This is called when we exit.
 * We call all the SPU's cleanup functions.
 */
static void stubSPUTearDown(void)
{
	/* shutdown, now trap any calls to a NULL dispatcher */
	crSPUCopyDispatchTable(&glim, &stubNULLDispatch);

	crSPUUnloadChain(stub.spu);
	stub.spu = NULL;

	crUnloadOpenGL();
}

static void stubExitHandler(void)
{
	stubSPUTearDown();
}

/**
 * Called when we receive a SIGTERM signal.
 */
static void stubSignalHandler(int signo)
{
	stubSPUTearDown();

	exit(0);  /* this causes stubExitHandler() to be called */
}


/**
 * Init variables in the stub structure, install signal handler.
 */
/**
 * Configuration options
 * NOTE: if you add any new options here, be sure to also add them to
 * the graphical config program in mothership/tools/crtypes.h in the
 * ApplicationNode class.
 */
static void stubInitVars(void)
{
	WindowInfo *defaultWin;

#ifdef CHROMIUM_THREADSAFE
	crInitMutex(&stub.mutex);
	crInitTSD(&stub.dispatchTSD);
#endif

	/* At the very least we want CR_RGB_BIT. */
	stub.haveNativeOpenGL = GL_FALSE;
	stub.spu = NULL;
	stub.appDrawCursor = 0;
	stub.minChromiumWindowWidth = 0;
	stub.minChromiumWindowHeight = 0;
	stub.maxChromiumWindowWidth = 0;
	stub.maxChromiumWindowHeight = 0;
	stub.matchChromiumWindowCount = 0;
	stub.matchChromiumWindowID = NULL;
	stub.matchWindowTitle = NULL;
	stub.ignoreFreeglutMenus = 1;
	stub.threadSafe = GL_FALSE;
	stub.trackWindowSize = 0;
	stub.trackWindowPos = 0;
	stub.trackWindowVisibility = 0;
/*	crSetenv( "CR_SYSTEM_GL_PATH", response ); */
	stub.force_pbuffers = 0;
	stub.spu_dir = NULL;

	stub.freeContextNumber = MAGIC_CONTEXT_BASE;
	stub.contextTable = crAllocHashtable();
	stub.currentContext = NULL;

	stub.windowTable = crAllocHashtable();

	defaultWin = (WindowInfo *) crCalloc(sizeof(WindowInfo));
	defaultWin->type = CHROMIUM;
	defaultWin->spuWindow = 0;  /* window 0 always exists */
	crHashtableAdd(stub.windowTable, 0, defaultWin);

	atexit(stubExitHandler);
	signal(SIGTERM, stubSignalHandler);
	signal(SIGINT, stubSignalHandler);
	signal(SIGPIPE, SIG_IGN); /* the networking code should catch this */
}

/**
 * Do one-time initializations for the library.
 */
void
stubInit(void)
{
	
	int num_spus;
	int spu_ids[3];
	char *spu_names[3];

	static int stub_initialized = 0;
	if (stub_initialized)
		return;
	stub_initialized = 1;
	
	stubInitVars();

	num_spus = 3;
	spu_ids[0] = 1;
	spu_ids[1] = 2;
	spu_ids[2] = 3;
	spu_names[0] = crStrdup( "array" );
	spu_names[1] = crStrdup( "feedback" );
	spu_names[2] = crStrdup( "pack" );

	stub.spu = crSPULoadChain( num_spus, spu_ids, spu_names, stub.spu_dir, NULL );

	crFree(spu_names[0]);
	crFree(spu_names[1]);
	crFree(spu_names[2]);

	crSPUInitDispatchTable( &glim );

	/* This is unlikely to change -- We still want to initialize our dispatch 
	 * table with the functions of the first SPU in the chain. */
	stubInitSPUDispatch( stub.spu );

	/* we need to plug one special stub function into the dispatch table */
	glim.GetChromiumParametervCR = stub_GetChromiumParametervCR;

	/* Load pointers to native OpenGL functions into stub.nativeDispatch */
	stubInitNativeDispatch();
}



#ifdef LINUX
/* GCC crap 
 *void (*stub_init_ptr)(void) __attribute__((section(".ctors"))) = __stubInit; */
#endif

