/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */


/*
 * How this all works...
 *
 * This directory implements two different interfaces to Chromium:
 *
 * 1. the Chromium interface - this is defined by the functions that start
 *    with the "cr" prefix and are defined in chromium.h and implemented in
 *    stub.c.  Typically, this is used by parallel apps (like psubmit).
 *
 * 2. GLX emulation interface - the glX*() functions are emulated here.
 *    When glXCreateContext() is called we may either create a real, native
 *    GLX context or a Chromium context (depending on match_window_title and
 *    mimimum_window_size).
 *
 */


#ifndef CR_STUB_H
#define CR_STUB_H

#include "chromium.h"
#include "cr_hash.h"
#include "cr_process.h"
#include "cr_spu.h"
#include "cr_threads.h"
#include "spu_dispatch_table.h"


/* When we first create a rendering context we can't be sure whether
 * it'll be handled by Chromium or as a native GLX context.  So in
 * CreateContext() we'll mark the ContextInfo object as UNDECIDED then
 * switch it to either NATIVE or CHROMIUM the first time MakeCurrent()
 * is called.  In MakeCurrent() we can use a criteria like window size
 * or window title to decide between CHROMIUM and NATIVE.
 */
typedef enum
{
	UNDECIDED,
	CHROMIUM,
	NATIVE
} ContextType;

#define MAX_DPY_NAME 1000

typedef struct context_info_t ContextInfo;
typedef struct window_info_t WindowInfo;

struct context_info_t
{
	char dpyName[MAX_DPY_NAME];
	GLint spuContext;  /* returned by head SPU's CreateContext() */
	ContextType type;  /* CHROMIUM, NATIVE or UNDECIDED */
	unsigned long id;          /* the client-visible handle */
	GLint visBits;
	WindowInfo *currentDrawable;
#if defined(GLX)
	Display *dpy;
	ContextInfo *share;
	XVisualInfo *visual;
	Bool direct;
	GLXContext glxContext;
#endif
};

struct window_info_t
{
	char dpyName[MAX_DPY_NAME];
	int x, y;
	unsigned int width, height;
	ContextType type;
	GLint spuWindow;       /* returned by head SPU's WindowCreate() */
	GLboolean mapped;
#if defined(GLX)
	Display *dpy;
	GLXDrawable drawable;
#endif
};

/* "Global" variables for the stub library */
typedef struct {
	/* the first SPU in the SPU chain on this app node */
	SPU *spu;

	/* OpenGL/SPU dispatch tables */
	crOpenGLInterface wsInterface;
	SPUDispatchTable spuDispatch;
	SPUDispatchTable nativeDispatch;
	GLboolean haveNativeOpenGL;

	/* config options */
	int appDrawCursor;
	GLuint minChromiumWindowWidth;
	GLuint minChromiumWindowHeight;
	GLuint maxChromiumWindowWidth;
	GLuint maxChromiumWindowHeight;
	GLuint matchChromiumWindowCount;
	GLuint matchChromiumWindowCounter;
	GLuint *matchChromiumWindowID;
	GLuint numIgnoreWindowID;
	char *matchWindowTitle;
	int ignoreFreeglutMenus;
	int trackWindowSize;
	int trackWindowPos;
	int trackWindowVisibility;
	char *spu_dir;
	int force_pbuffers;

	/* thread safety stuff */
	GLboolean threadSafe;
#ifdef CHROMIUM_THREADSAFE
	CRtsd dispatchTSD;
	CRmutex mutex;
#endif

	/* contexts */
	int freeContextNumber;
	CRHashTable *contextTable;
	ContextInfo *currentContext; /* may be NULL */

	/* windows */
	CRHashTable *windowTable;
} Stub;


extern Stub stub;
extern SPUDispatchTable glim;
extern SPUDispatchTable stubThreadsafeDispatch;
extern SPUDispatchTable stubNULLDispatch;


#if defined(GLX)

/* GLX versions */
extern WindowInfo *stubGetWindowInfo( Display *dpy, GLXDrawable drawable );
extern void stubUseXFont( Display *dpy, Font font, int first, int count, int listbase );

#endif


extern ContextInfo *stubNewContext( const char *dpyName, GLint visBits, ContextType type, unsigned long shareCtx );
extern void stubDestroyContext( unsigned long contextId );
extern GLboolean stubMakeCurrent( WindowInfo *window, ContextInfo *context );
extern GLint stubNewWindow( const char *dpyName, GLint visBits );
extern void stubSwapBuffers( const WindowInfo *window, GLint flags );
extern void stubGetWindowGeometry( const WindowInfo *win, int *x, int *y, unsigned int *w, unsigned int *h );
extern GLboolean stubIsWindowVisible( const WindowInfo *win );
extern void stubInit(void);

extern void APIENTRY stub_GetChromiumParametervCR( GLenum target, GLuint index, GLenum type, GLsizei count, GLvoid *values );

extern void APIENTRY glBoundsInfoCR(const CRrecti *, const GLbyte *, GLint, GLint);

#endif /* CR_STUB_H */
