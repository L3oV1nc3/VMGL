/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */
#ifndef CR_SPU_H
#define CR_SPU_H

#define SPULOAD_APIENTRY

#include "cr_dll.h"
#include "spu_dispatch_table.h"
#include "cr_net.h"

#define SPU_ENTRY_POINT_NAME "SPULoad"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_THREADS               32      /**< max threads per spu */

typedef struct _SPUSTRUCT SPU;

typedef void (*SPUGenericFunction)(void);

/**
 * SPU Named function descriptor
 */
typedef struct {
	char *name;
	SPUGenericFunction fn;
} SPUNamedFunctionTable;

/**
 * SPU function table descriptor
 */
typedef struct {
	SPUDispatchTable *childCopy;
	void *data;
	SPUNamedFunctionTable *table;
} SPUFunctions;

/** 
 * SPU Option callback
 * \param spu
 * \param response
 */
typedef void (*SPUOptionCB)( void *spu, const char *response );

typedef enum { CR_BOOL, CR_INT, CR_FLOAT, CR_STRING, CR_ENUM } cr_type;

/**
 * SPU Options table
 */
typedef struct {
	const char *option;	/**< Name of the option */
	cr_type type; 		/**< Type of option */
	int numValues;  /**< usually 1 */
	const char *deflt;  /**< comma-separated string of [numValues] defaults */
	const char *min;    /**< comma-separated string of [numValues] minimums */
	const char *max;    /**< comma-separated string of [numValues] maximums */
	const char *description; /**< Textual description of the option */
	SPUOptionCB cb;		/**< Callback function */
} SPUOptions, *SPUOptionsPtr;


/** Init spu */
typedef SPUFunctions *(*SPUInitFuncPtr)(int id, SPU *child,
		SPU *super, unsigned int, unsigned int );
typedef void (*SPUSelfDispatchFuncPtr)(SPUDispatchTable *);
/** Cleanup spu */
typedef int (*SPUCleanupFuncPtr)(void);
/** Load spu */
typedef int (*SPULoadFunction)(char **, char **, void *, void *, void *,
			       SPUOptionsPtr *, int *);


/**
 * masks for spu_flags 
 */
#define SPU_PACKER_MASK           0x1
#define SPU_NO_PACKER             0x0
#define SPU_HAS_PACKER            0x1
#define SPU_TERMINAL_MASK         0x2
#define SPU_NOT_TERMINAL          0x0
#define SPU_IS_TERMINAL           0x2
#define SPU_MAX_SERVERS_MASK      0xc
#define SPU_MAX_SERVERS_ZERO      0x0
#define SPU_MAX_SERVERS_ONE       0x4
#define SPU_MAX_SERVERS_UNLIMITED 0x8


/**
 * SPU descriptor
 */
struct _SPUSTRUCT {
	char *name;			/**< Name of the spu */
	char *super_name;		/**< Name of the super class of the spu */
	int id;				/**< Id num of the spu */
        int spu_flags;			/**< options fags for the SPU */
	struct _SPUSTRUCT *superSPU;	/**< Pointer to the descriptor for the super class */
	CRDLL *dll;			/**< pointer to shared lib for spu */
	SPULoadFunction entry_point;	/**< SPU's entry point (SPULoad()) */
	SPUInitFuncPtr init;		/**< SPU init function */
	SPUSelfDispatchFuncPtr self;	/**< */
	SPUCleanupFuncPtr cleanup;	/**< SPU cleanup func */
	SPUFunctions *function_table;	/**< Function table for spu */
	SPUOptions *options;		/**< Options table */
	SPUDispatchTable dispatch_table;
	void *privatePtr;  		/**< pointer to SPU-private data */
};


/**
 * These are the OpenGL / window system interface functions
 */

/**
 * X11/GLX
 */
typedef int (*glXGetConfigFunc_t)( Display *, XVisualInfo *, int, int * );
typedef Bool (*glXQueryExtensionFunc_t) (Display *, int *, int * );
typedef const char *(*glXQueryExtensionsStringFunc_t) (Display *, int );
typedef Bool (*glXQueryVersionFunc_t)( Display *dpy, int *maj, int *min );
typedef XVisualInfo *(*glXChooseVisualFunc_t)( Display *, int, int * );
typedef GLXContext (*glXCreateContextFunc_t)( Display *, XVisualInfo *, GLXContext, Bool );
typedef void (*glXUseXFontFunc_t)(Font font, int first, int count, int listBase);
typedef void (*glXDestroyContextFunc_t)( Display *, GLXContext );
typedef Bool (*glXIsDirectFunc_t)( Display *, GLXContext );
typedef Bool (*glXMakeCurrentFunc_t)( Display *, GLXDrawable, GLXContext );
typedef void (*glXSwapBuffersFunc_t)( Display *, GLXDrawable );
typedef CR_GLXFuncPtr (*glXGetProcAddressARBFunc_t)( const GLubyte *name );
typedef Display *(*glXGetCurrentDisplayFunc_t)( void );
typedef GLXContext (*glXGetCurrentContextFunc_t)( void );
typedef GLXDrawable (*glXGetCurrentDrawableFunc_t)( void );
typedef char * (*glXGetClientStringFunc_t)( Display *dpy, int name );
typedef void (*glXWaitGLFunc_t)(void);
typedef void (*glXWaitXFunc_t)(void);
typedef void (*glXCopyContextFunc_t)(Display *dpy, GLXContext src, GLXContext dst, unsigned long mask );
typedef const GLubyte *(*glGetStringFunc_t)( GLenum );
typedef Bool (*glXJoinSwapGroupNVFunc_t)(Display *dpy, GLXDrawable drawable, GLuint group);
typedef Bool (*glXBindSwapBarrierNVFunc_t)(Display *dpy, GLuint group, GLuint barrier);
typedef Bool (*glXQuerySwapGroupNVFunc_t)(Display *dpy, GLXDrawable drawable, GLuint *group, GLuint *barrier);
typedef Bool (*glXQueryMaxSwapGroupsNVFunc_t)(Display *dpy, int screen, GLuint *maxGroups, GLuint *maxBarriers);
typedef Bool (*glXQueryFrameCountNVFunc_t)(Display *dpy, int screen, GLuint *count);
typedef Bool (*glXResetFrameCountNVFunc_t)(Display *dpy, int screen);
#ifdef GLX_VERSION_1_3
typedef GLXContext (*glXCreateNewContextFunc_t)( Display *dpy, GLXFBConfig config, int renderType, GLXContext shareList, Bool direct );
typedef GLXWindow (*glXCreateWindowFunc_t)(Display *dpy, GLXFBConfig config, Window win, const int *attrib_list);
typedef Bool (*glXMakeContextCurrentFunc_t)( Display *dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx );
typedef GLXFBConfig *(*glXChooseFBConfigFunc_t)( Display *dpy, int screen, const int *attribList, int *nitems );
typedef GLXFBConfig *(*glXGetFBConfigsFunc_t)(Display *dpy, int screen, int *nelements);
typedef int (*glXGetFBConfigAttribFunc_t)(Display *dpy, GLXFBConfig config, int attribute, int *value);
typedef XVisualInfo *(*glXGetVisualFromFBConfigFunc_t)(Display *dpy, GLXFBConfig config);
typedef GLXPbuffer (*glXCreatePbufferFunc_t)( Display *dpy, GLXFBConfig config, const int *attribList );
typedef void (*glXDestroyPbufferFunc_t)( Display *dpy, GLXPbuffer pbuf );
typedef int (*glXQueryContextFunc_t)(Display *dpy, GLXContext ctx, int attribute, int *value);
typedef void (*glXQueryDrawableFunc_t)(Display *dpy, GLXDrawable draw, int attribute, unsigned int *value);
#endif /* GLX_VERSION_1_3 */


/**
 * Package up the GLX function pointers into a struct.  We use
 * this in a few different places.
 */
typedef struct {
	glXGetConfigFunc_t  glXGetConfig;
	glXQueryExtensionFunc_t glXQueryExtension;
	glXQueryVersionFunc_t glXQueryVersion;
	glXQueryExtensionsStringFunc_t glXQueryExtensionsString;
	glXChooseVisualFunc_t glXChooseVisual;
	glXCreateContextFunc_t glXCreateContext;
	glXDestroyContextFunc_t glXDestroyContext;
	glXUseXFontFunc_t glXUseXFont;
	glXIsDirectFunc_t glXIsDirect;
	glXMakeCurrentFunc_t glXMakeCurrent;
	glXSwapBuffersFunc_t glXSwapBuffers;
	glXGetProcAddressARBFunc_t glXGetProcAddressARB;
	glXGetCurrentDisplayFunc_t glXGetCurrentDisplay;
	glXGetCurrentContextFunc_t glXGetCurrentContext;
	glXGetCurrentDrawableFunc_t glXGetCurrentDrawable;
	glXGetClientStringFunc_t glXGetClientString;
	glXWaitGLFunc_t glXWaitGL;
	glXWaitXFunc_t glXWaitX;
	glXCopyContextFunc_t glXCopyContext;
	/* GLX_NV_swap_group */
	glXJoinSwapGroupNVFunc_t glXJoinSwapGroupNV;
	glXBindSwapBarrierNVFunc_t glXBindSwapBarrierNV;
	glXQuerySwapGroupNVFunc_t glXQuerySwapGroupNV;
	glXQueryMaxSwapGroupsNVFunc_t glXQueryMaxSwapGroupsNV;
	glXQueryFrameCountNVFunc_t glXQueryFrameCountNV;
	glXResetFrameCountNVFunc_t glXResetFrameCountNV;
#ifdef GLX_VERSION_1_3
	glXCreateNewContextFunc_t glXCreateNewContext;
	glXCreateWindowFunc_t glXCreateWindow;
	glXMakeContextCurrentFunc_t glXMakeContextCurrent;
	glXChooseFBConfigFunc_t glXChooseFBConfig;
	glXGetFBConfigsFunc_t glXGetFBConfigs;
	glXGetFBConfigAttribFunc_t glXGetFBConfigAttrib;
	glXGetVisualFromFBConfigFunc_t glXGetVisualFromFBConfig;
	glXCreatePbufferFunc_t glXCreatePbuffer;
	glXDestroyPbufferFunc_t glXDestroyPbuffer;
	glXQueryContextFunc_t glXQueryContext;
	glXQueryDrawableFunc_t glXQueryDrawable;
#endif
	glGetStringFunc_t glGetString;
} crOpenGLInterface;


/** This is the one required function in _all_ SPUs */
int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options, int *flags );

SPU *crSPULoad( SPU *child, int id, char *name, char *dir, void *server);
SPU *crSPULoadChain( int count, int *ids, char **names, char *dir, void *server );
void crSPUUnloadChain(SPU *headSPU);

void crSPUInitDispatchTable( SPUDispatchTable *table );
void crSPUCopyDispatchTable( SPUDispatchTable *dst, SPUDispatchTable *src );
void crSPUChangeInterface( SPUDispatchTable *table, void *origFunc, void *newFunc );


void crSPUSetDefaultParams( void *spu, SPUOptions *options );
int crSPUGetEnumIndex( const SPUOptions *option, const char *optName, const char *value );


SPUGenericFunction crSPUFindFunction( const SPUNamedFunctionTable *table, const char *fname );
void crSPUInitDispatch( SPUDispatchTable *dispatch, const SPUNamedFunctionTable *table );
void crSPUInitDispatchNops(SPUDispatchTable *table);

int crLoadOpenGL( crOpenGLInterface *crInterface, SPUNamedFunctionTable table[] );
void crUnloadOpenGL( void );
int crLoadOpenGLExtensions( const crOpenGLInterface *crInterface, SPUNamedFunctionTable table[] );

#if defined(GLX)
XVisualInfo *
crChooseVisual(const crOpenGLInterface *ws, Display *dpy, int screen,
	       GLboolean directColor, int visBits);
#else
int
crChooseVisual(const crOpenGLInterface *ws, int visBits);
#endif


#ifdef USE_OSMESA
int crLoadOSMesa( OSMesaContext (**createContext)( GLenum format, OSMesaContext sharelist ), 
		  GLboolean (**makeCurrent)( OSMesaContext ctx, GLubyte *buffer, 
					     GLenum type, GLsizei width, GLsizei height ),
		  void (**destroyContext)( OSMesaContext ctx ));
#endif

#ifdef __cplusplus
}
#endif

#endif /* CR_SPU_H */
