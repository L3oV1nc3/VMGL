/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

/**
 * \mainpage OpenGL_stub 
 *
 * \section OpenGL_stubIntroduction Introduction
 *
 * Chromium consists of all the top-level files in the cr
 * directory.  The OpenGL_stub module basically takes care of API dispatch,
 * and OpenGL state management.
 *
 */

/**
 * This file manages OpenGL rendering contexts in the VMGL library.
 * The big issue is switching between Chromium and native GL context
 * management.  This is where we support multiple client OpenGL
 * windows.  Typically, one window is handled by Chromium while any
 * other windows are handled by the native OpenGL library.
 * For VMGL this is not a worry. Kept for historical reasons.
 */

#include "chromium.h"
#include "cr_error.h"
#include "cr_spu.h"
#include "cr_mem.h"
#include "cr_string.h"
#include "cr_environment.h"
#include "stub.h"


/**
 * This function should be called from MakeCurrent().  It'll detect if
 * we're in a multi-thread situation, and do the right thing for dispatch.
 */
#ifdef CHROMIUM_THREADSAFE
    static void
stubCheckMultithread( void )
{
    static CRthread knownID;
    static GLboolean firstCall = GL_TRUE;

    if (stub.threadSafe)
	return;  /* nothing new, nothing to do */

    if (firstCall) {
	knownID = crThreadID();
	firstCall = GL_FALSE;
    }
    else if (knownID != crThreadID()) {
	/* going thread-safe now! */
	stub.threadSafe = GL_TRUE;
	crSPUCopyDispatchTable(&glim, &stubThreadsafeDispatch);
    }
}
#endif


/**
 * Install the given dispatch table as the table used for all gl* calls.
 */
    static void
stubSetDispatch( SPUDispatchTable *table )
{
    CRASSERT(table);

#ifdef CHROMIUM_THREADSAFE
    /* always set the per-thread dispatch pointer */
    crSetTSD(&stub.dispatchTSD, (void *) table);
    if (stub.threadSafe) {
	/* Do nothing - the thread-safe dispatch functions will call GetTSD()
	 * to get a pointer to the dispatch table, and jump through it.
	 */
    }
    else 
#endif
    {
	/* Single thread mode - just install the caller's dispatch table */
	/* This conditional is an optimization to try to avoid unnecessary
	 * copying.  It seems to work with atlantis, multiwin, etc. but
	 * _could_ be a problem. (Brian)
	 */
	if (glim.copy_of != table->copy_of)
	    crSPUCopyDispatchTable(&glim, table);
    }
}


/**
 * Create a new _Chromium_ window, not GLX.
 * Called by crWindowCreate() only.
 */
    GLint
stubNewWindow( const char *dpyName, GLint visBits )
{
    WindowInfo *winInfo;
    GLint spuWin, size[2];

    spuWin = stub.spu->dispatch_table.WindowCreate( dpyName, visBits );
    if (spuWin < 0) {
	return -1;
    }

    winInfo = (WindowInfo *) crCalloc(sizeof(WindowInfo));
    if (!winInfo) {
	stub.spu->dispatch_table.WindowDestroy(spuWin);
	return -1;
    }

    winInfo->type = CHROMIUM;

    /* Ask the head SPU for the initial window size */
    size[0] = size[1] = 0;
    stub.spu->dispatch_table.GetChromiumParametervCR(GL_WINDOW_SIZE_CR, 0, GL_INT, 2, size);
    if (size[0] == 0 && size[1] == 0) {
	/* use some reasonable defaults */
	size[0] = size[1] = 512;
    }
    winInfo->width = size[0];
    winInfo->height = size[1];
    winInfo->mapped = 1;

    if (!dpyName)
	dpyName = "";

    crStrncpy(winInfo->dpyName, dpyName, MAX_DPY_NAME);
    winInfo->dpyName[MAX_DPY_NAME-1] = 0;

    /* Use spuWin as the hash table index and GLX handle */
#if defined(GLX)
    winInfo->drawable = (GLXDrawable) spuWin;
#endif
    winInfo->spuWindow = spuWin;

    crHashtableAdd(stub.windowTable, (unsigned int) spuWin, winInfo);

    return spuWin;
}


    GLboolean
stubIsWindowVisible( const WindowInfo *win )
{
#if defined(GLX)
    if (win->dpy) {
	XWindowAttributes attr;
	XGetWindowAttributes(win->dpy, win->drawable, &attr);
	return (attr.map_state != IsUnmapped);
    }
    else {
	/* probably created by crWindowCreate() */
	return win->mapped;
    }
#endif
}


/**
 * Given a Windows HDC or GLX Drawable, return the corresponding
 * WindowInfo structure.  Create a new one if needed.
 */
WindowInfo *
#if defined(GLX)
stubGetWindowInfo( Display *dpy, GLXDrawable drawable )
#endif
{
    WindowInfo *winInfo = (WindowInfo *) crHashtableSearch(stub.windowTable, (unsigned int) drawable);
    if (!winInfo) {
	winInfo = (WindowInfo *) crCalloc(sizeof(WindowInfo));
	if (!winInfo)
	    return NULL;
#ifdef GLX
	crStrncpy(winInfo->dpyName, DisplayString(dpy), MAX_DPY_NAME);
	winInfo->dpyName[MAX_DPY_NAME-1] = 0;
	winInfo->dpy = dpy;
#endif
	winInfo->drawable = drawable;
	winInfo->type = UNDECIDED;
	winInfo->spuWindow = -1;
	winInfo->mapped = -1; /* don't know */
	crHashtableAdd(stub.windowTable, (unsigned int) drawable, winInfo);
    }
    return winInfo;
}


/**
 * Allocate a new ContextInfo object, initialize it, put it into the
 * context hash table.  If type==CHROMIUM, call the head SPU's
 * CreateContext() function too.
 */
    ContextInfo *
stubNewContext( const char *dpyName, GLint visBits, ContextType type,
	unsigned long shareCtx )
{
    GLint spuContext = -1, spuShareCtx = 0;
    ContextInfo *context;

    if (shareCtx > 0) {
	/* translate shareCtx to a SPU context ID */
	context = (ContextInfo *)
	    crHashtableSearch(stub.contextTable, shareCtx);
	if (context)
	    spuShareCtx = context->spuContext;
    }

    if (type == CHROMIUM) {
	spuContext
		= stub.spu->dispatch_table.CreateContext(dpyName, visBits, spuShareCtx);
	if (spuContext < 0)
		return NULL;
    }

    context = crCalloc(sizeof(ContextInfo));
    if (!context) {
	stub.spu->dispatch_table.DestroyContext(spuContext);
	return NULL;
    }

    if (!dpyName)
	dpyName = "";

    context->id = stub.freeContextNumber++;
    context->type = type;
    context->spuContext = spuContext;
    context->visBits = visBits;
    context->currentDrawable = NULL;
    crStrncpy(context->dpyName, dpyName, MAX_DPY_NAME);
    context->dpyName[MAX_DPY_NAME-1] = 0;

#if defined(GLX)
    context->share = (ContextInfo *)
    	crHashtableSearch(stub.contextTable, (unsigned long) shareCtx);
#endif

    crHashtableAdd(stub.contextTable, context->id, (void *) context);

    return context;
}


/**
 * This creates a native GLX context.
 */
static GLboolean
InstantiateNativeContext( WindowInfo *window, ContextInfo *context )
{
#if defined(GLX)
	GLXContext shareCtx = 0;

	/* sort out context sharing here */
	if (context->share) {
			if (context->glxContext != context->share->glxContext) {
					crWarning("glXCreateContext() is trying to share a non-existant "
										"GLX context.  Setting share context to zero.");
					shareCtx = 0;
			}
			else {
					shareCtx = context->glxContext;
			}
	}

	context->glxContext = stub.wsInterface.glXCreateContext( window->dpy,
				 context->visual, shareCtx, context->direct );

	return context->glxContext ? GL_TRUE : GL_FALSE;
#endif
}


/**
 * Utility functions to get window size and titlebar text.
 */
#if defined(GLX)

void
stubGetWindowGeometry( const WindowInfo *window, int *x, int *y,
											 unsigned int *w, unsigned int *h )
{
	Window root, child;
	unsigned int border, depth;
	if (!window
			|| !window->dpy
			|| !window->drawable
			|| !XGetGeometry(window->dpy, window->drawable, &root,
											 x, y, w, h, &border, &depth)
			|| !XTranslateCoordinates(window->dpy, window->drawable, root,
																*x, *y, x, y, &child)) {
		*x = *y = 0;
		*w = *h = 0;
	}
}

static char *
GetWindowTitleHelper( Display *dpy, Window window, GLboolean recurseUp )
{
	while (1) {
		char *name;
		if (!XFetchName(dpy, window, &name))
			return NULL;
		if (name[0]) {
			return name;
		}
		else if (recurseUp) {
			/* This window has no name, try the parent */
			Status stat;
			Window root, parent, *children;
			unsigned int numChildren;
			stat = XQueryTree( dpy, window, &root, &parent,
												 &children, &numChildren );
			if (!stat || window == root)
				return NULL;
			if (children)
				XFree(children);
			window = parent;
		}
		else {
			XFree(name);
			return NULL;
		}
	}
}

static void
GetWindowTitle( const WindowInfo *window, char *title )
{
	char *t = GetWindowTitleHelper(window->dpy, window->drawable, GL_TRUE);
	if (t) {
		crStrcpy(title, t);
		XFree(t);
	}
	else {
		title[0] = 0;
	}
}


/**
 *Return current cursor position in local window coords.
 */
static void
GetCursorPosition( const WindowInfo *window, int pos[2] )
{
	int rootX, rootY;
	Window root, child;
	unsigned int mask;
	int x, y;
	Bool q = XQueryPointer(window->dpy, window->drawable, &root, &child,
												 &rootX, &rootY, &pos[0], &pos[1], &mask);
	if (q) {
		unsigned int w, h;
		stubGetWindowGeometry( window, &x, &y, &w, &h );
		/* invert Y */
		pos[1] = (int) h - pos[1] - 1;
	}
	else {
		pos[0] = pos[1] = 0;
	}
}

#endif


/**
 * This function is called by MakeCurrent() and determines whether or
 * not a new rendering context should be bound to Chromium or the native
 * OpenGL.
 * \return  GL_FALSE if native OpenGL should be used, or GL_TRUE if Chromium
 *          should be used.
 */
static GLboolean
stubCheckUseChromium( WindowInfo *window )
{
	int x, y;
	unsigned int w, h;

	/* If the provided window is CHROMIUM, we're clearly intended
	 * to create a CHROMIUM context.
	 */
	if (window->type == CHROMIUM)
		return GL_TRUE;

	if (stub.ignoreFreeglutMenus) {
		const char *glutMenuTitle = "freeglut menu";
		char title[1000];
		GetWindowTitle(window, title);
		if (crStrcmp(title, glutMenuTitle) == 0) {
			crDebug("Ignoring freeglut menu window");
			return GL_FALSE;
		}
	}

	/*  If the user's specified a window count for Chromium, see if
		*  this window satisfies that criterium.
		*/
	stub.matchChromiumWindowCounter++;
	if (stub.matchChromiumWindowCount > 0) {
		if (stub.matchChromiumWindowCounter != stub.matchChromiumWindowCount) {
			crDebug("Using native GL, app window doesn't meet match_window_count");
			return GL_FALSE;
		}
	}

	/* If the user's specified a window list to ignore, see if this
	 * window satisfies that criterium.
	 */
	if (stub.matchChromiumWindowID) {
		GLuint i;

		for (i = 0; i <= stub.numIgnoreWindowID; i++) {
			if (stub.matchChromiumWindowID[i] == stub.matchChromiumWindowCounter) {
				crDebug("Ignore window ID %d, using native GL", stub.matchChromiumWindowID[i]);
				return GL_FALSE;
			}
		}
	}

	/* If the user's specified a minimum window size for Chromium, see if
	 * this window satisfies that criterium. 
	 */
	if (stub.minChromiumWindowWidth > 0 && 
	    stub.minChromiumWindowHeight > 0) {
		stubGetWindowGeometry( window, &x, &y, &w, &h );
		if (w >= stub.minChromiumWindowWidth && 
		    h >= stub.minChromiumWindowHeight) {

			/* Check for maximum sized window now too */
			if (stub.maxChromiumWindowWidth && 
			    stub.maxChromiumWindowHeight) {
				if (w < stub.maxChromiumWindowWidth &&
				    h < stub.maxChromiumWindowHeight)
					return GL_TRUE;
				else 
					return GL_FALSE;
			}

			return GL_TRUE;
		}
		crDebug("Using native GL, app window doesn't meet minimum_window_size");
		return GL_FALSE;
	}
	else if (stub.matchWindowTitle) {
		/* If the user's specified a window title for Chromium, see if this
		 * window satisfies that criterium.
		 */
		GLboolean wildcard = GL_FALSE;
		char title[1000];
		char *titlePattern;
		int len;
		/* check for leading '*' wildcard */
		if (stub.matchWindowTitle[0] == '*') {
			titlePattern = crStrdup( stub.matchWindowTitle + 1 );
			wildcard = GL_TRUE;
		}
		else {
			titlePattern = crStrdup( stub.matchWindowTitle );
		}
		/* check for trailing '*' wildcard */
		len = crStrlen(titlePattern);
		if (len > 0 && titlePattern[len - 1] == '*') {
			titlePattern[len - 1] = '\0'; /* terminate here */
			wildcard = GL_TRUE;
		}

		GetWindowTitle( window, title );
		if (title[0]) {
			if (wildcard) {
				if (crStrstr(title, titlePattern)) {
					crFree(titlePattern);
					return GL_TRUE;
				}
			}
			else if (crStrcmp(title, titlePattern) == 0) {
				crFree(titlePattern);
				return GL_TRUE;
			}
		}
		crFree(titlePattern);
		crDebug("Using native GL, app window title doesn't match match_window_title string (\"%s\" != \"%s\")", title, stub.matchWindowTitle);
		return GL_FALSE;
	}

	/* Window title and size don't matter */
	CRASSERT(stub.minChromiumWindowWidth == 0);
	CRASSERT(stub.minChromiumWindowHeight == 0);
	CRASSERT(stub.matchWindowTitle == NULL);

	/* User hasn't specified a width/height or window title.
	 * We'll use chromium for this window (and context) if no other is.
	 */

	return GL_TRUE;  /* use Chromium! */
}


GLboolean
stubMakeCurrent( WindowInfo *window, ContextInfo *context )
{
	GLboolean retVal;

	/*
	 * Get WindowInfo and ContextInfo pointers.
	 */

	if (!context || !window) {
		if (stub.currentContext)
			stub.currentContext->currentDrawable = NULL;
		if (context)
			context->currentDrawable = NULL;
		stub.currentContext = NULL;
		return GL_TRUE;  /* OK */
	}

#ifdef CHROMIUM_THREADSAFE
	stubCheckMultithread();
#endif

	if (context->type == UNDECIDED) {
		/* Here's where we really create contexts */
#ifdef CHROMIUM_THREADSAFE
		crLockMutex(&stub.mutex);
#endif

		if (stubCheckUseChromium(window)) {
			/*
			 * Create a Chromium context.
			 */
#if defined(GLX) || defined(DARWIN)
			GLint spuShareCtx = context->share ? context->share->spuContext : 0;
#else
      GLint spuShareCtx = 0;
#endif

			CRASSERT(stub.spu);
			CRASSERT(stub.spu->dispatch_table.CreateContext);
			context->type = CHROMIUM;

			context->spuContext
				= stub.spu->dispatch_table.CreateContext( context->dpyName,context->visBits,spuShareCtx );
			if (window->spuWindow == -1)
				window->spuWindow = stub.spu->dispatch_table.WindowCreate( window->dpyName, context->visBits );
		}
		else {
			/*
			 * Create a native OpenGL context.
			 */
			if (!InstantiateNativeContext(window, context))
			{
#ifdef CHROMIUM_THREADSAFE
				crUnlockMutex(&stub.mutex);
#endif
				return 0; /* false */
			}
			context->type = NATIVE;
		}

#ifdef CHROMIUM_THREADSAFE
		crUnlockMutex(&stub.mutex);
#endif
	}


	if (context->type == NATIVE) {
		/*
		 * Native OpenGL MakeCurrent().
		 */
#if defined(GLX)
		retVal = (GLboolean) stub.wsInterface.glXMakeCurrent( window->dpy, window->drawable, context->glxContext );
#endif
	}
	else {
		/*
		 * SPU chain MakeCurrent().
		 */
		CRASSERT(context->type == CHROMIUM);
		CRASSERT(context->spuContext >= 0);

		if (context->currentDrawable && context->currentDrawable != window)
			crWarning("Rebinding context %p to a different window", context);

		if (window->type == NATIVE) {
			crWarning("Can't rebind a chromium context to a native window\n");
			retVal = 0;
		}
		else {
			if (window->spuWindow == -1)
				window->spuWindow = stub.spu->dispatch_table.WindowCreate( window->dpyName, context->visBits );

			if (window->spuWindow != (GLint)window->drawable)
				 stub.spu->dispatch_table.MakeCurrent( window->spuWindow, (GLint) window->drawable, context->spuContext );
			else
				 stub.spu->dispatch_table.MakeCurrent( window->spuWindow, 0, /* native window handle */ context->spuContext );

			retVal = 1;
		}
	}

	window->type = context->type;
	context->currentDrawable = window;
	stub.currentContext = context;

	if (retVal) {
		/* Now, if we've transitions from Chromium to native rendering, or
		 * vice versa, we have to change all the OpenGL entrypoint pointers.
		 */
		if (context->type == NATIVE) {
			/* Switch to native API */
			/*printf("  Switching to native API\n");*/
			stubSetDispatch(&stub.nativeDispatch);
		}
		else if (context->type == CHROMIUM) {
			/* Switch to stub (SPU) API */
			/*printf("  Switching to spu API\n");*/
			stubSetDispatch(&stub.spuDispatch);
		}
		else {
			/* no API switch needed */
		}
	}

	if (!window->width && window->type == CHROMIUM) {
		/* One time window setup */
		int x, y;
		unsigned int winW, winH;

		stubGetWindowGeometry( window, &x, &y, &winW, &winH );

		/* If we're not using GLX (no app window) we'll always get
		 * a width and height of zero here.  In that case, skip the viewport
		 * call since we're probably using a tilesort SPU with fake_window_dims
		 * which the tilesort SPU will use for the viewport.
		 */
		window->width = winW;
		window->height = winH;
		if (stub.trackWindowSize)
			stub.spuDispatch.WindowSize( window->spuWindow, winW, winH );
		if (winW > 0 && winH > 0)
			stub.spu->dispatch_table.Viewport( 0, 0, winW, winH );
	}

	/* Update window mapping state.
	 * Basically, this lets us hide render SPU windows which correspond
	 * to unmapped application windows.  Without this, perfly (for example)
	 * opens *lots* of temporary windows which otherwise clutter the screen.
	 */
	if (stub.trackWindowVisibility && window->type == CHROMIUM && window->drawable) {
		const int mapped = stubIsWindowVisible(window);
		if (mapped != window->mapped) {
			stub.spu->dispatch_table.WindowShow(window->spuWindow, mapped);
			window->mapped = mapped;
		}
	}

	return retVal;
}



void
stubDestroyContext( unsigned long contextId )
{
	ContextInfo *context;

	context = (ContextInfo *) crHashtableSearch(stub.contextTable, contextId);

	CRASSERT(context);

	if (context->type == NATIVE) {
#if defined(GLX)
		stub.wsInterface.glXDestroyContext( context->dpy, context->glxContext );
#endif
	}
	else if (context->type == CHROMIUM) {
		/* Have pack SPU or tilesort SPU, etc. destroy the context */
		CRASSERT(context->spuContext >= 0);
		stub.spu->dispatch_table.DestroyContext( context->spuContext );
	}

	if (stub.currentContext == context) {
		stub.currentContext = NULL;
	}

	crMemZero(context, sizeof(ContextInfo));  /* just to be safe */
	crHashtableDelete(stub.contextTable, contextId, crFree);
}


void
stubSwapBuffers( const WindowInfo *window, GLint flags )
{
	if (!window)
		return;

	/* Determine if this window is being rendered natively or through
	 * Chromium.
	 */

	if (window->type == NATIVE) {
		/*printf("*** Swapping native window %d\n", (int) drawable);*/
#if defined(GLX)
		stub.wsInterface.glXSwapBuffers( window->dpy, window->drawable );
#endif
	}
	else if (window->type == CHROMIUM) {
		/* Let the SPU do the buffer swap */
		/*printf("*** Swapping chromium window %d\n", (int) drawable);*/
		if (stub.appDrawCursor) {
			int pos[2];
			GetCursorPosition(window, pos);
			stub.spu->dispatch_table.ChromiumParametervCR(GL_CURSOR_POSITION_CR, GL_INT, 2, pos);
		}
		stub.spu->dispatch_table.SwapBuffers( window->spuWindow, flags );
	}
	else {
		crDebug("Calling SwapBuffers on a window we haven't seen before (no-op).");
	}
}
