/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_PACKSPU_H
#define CR_PACKSPU_H

#define PACKSPU_APIENTRY

#include "cr_glstate.h"
#include "cr_netserver.h"
#include "cr_pack.h"
#include "cr_spu.h"
#include "cr_threads.h"
#include "cr_hash.h"
#include "state/cr_client.h"
#include "cr_list.h"
#include "cr_dlm.h"

typedef struct thread_info_t ThreadInfo;
typedef struct context_info_t ContextInfo;
typedef struct window_info_t WindowInfo;

struct window_info_t {
	GLint XWindow;
        GLint visual;
};

struct context_info_t {
	CRContext *clientState;  /* used to store client-side GL state */
	GLint serverCtx;         /* context ID returned by server */
	char glVersion[100];     /* GL_VERSION string */
	/* resume state needs */
        GLint shareCtx;
        GLint visual;
        GLint boundWindow;
	/* DLM for resume state */
        CRDLM *DLM;
        CRDLMContextState *DLMState;
        GLenum DL_mode;
        GLuint DL_ID;
};

struct thread_info_t {
	CRthread id;
	CRNetServer netServer;
	CRPackBuffer buffer;
	CRPackBuffer normBuffer;
	CRPackBuffer BeginEndBuffer;
	GLenum BeginEndMode;
	int BeginEndState;
	ContextInfo *currentContext;
	CRPackContext *packer;
	int writeback;
};

typedef struct {
	int id;
	int swap;

	/* config options */
	int emit_GATHER_POST_SWAPBUFFERS;
	int swapbuffer_sync;

	int ReadPixels;

	char *name;
	int buffer_size;

	int numThreads;
	ThreadInfo thread[MAX_THREADS];

	int numContexts;
	ContextInfo context[CR_MAX_CONTEXTS];
	
	/* VMGL: */	
	in_port_t secondPort;
	in_addr_t serverIP;
	CRHashTable *XWindows;
	Display *XDisplay;
	int openedXDisplay;	
	/* State resume */
	SPUDispatchTable self;
	SPUDispatchTable diff_DT;
	SPUDispatchTable state_DT;
	/* Necessary for orderly context state resume */
	CRList *contextList;

	/* Hackish. This flag indicates w have to enter
	 * a suspend/resume cycle after the next SwapBuffers 
	 * call is completed */
	int doResume;

} PackSPU;

extern PackSPU pack_spu;

#ifdef CHROMIUM_THREADSAFE
extern CRmutex _PackMutex;
extern CRtsd _PackTSD;
#define GET_THREAD(T)  ThreadInfo *T = crGetTSD(&_PackTSD)
#else
#define GET_THREAD(T)  ThreadInfo *T = &(pack_spu.thread[0])
#endif

#define GET_CONTEXT(C)                      \
  GET_THREAD(thread);                       \
  ContextInfo *C = thread->currentContext

#define IN_DL(thread) ((thread)->currentContext->DL_mode != GL_FALSE)
#define NOT_COMPILE_DL(thread) ((thread)->currentContext->DL_mode != GL_COMPILE)

#define PACKSPU_CONNECT_RETRIES 5
#define PACKSPU_CONNECT_RETRY_SLEEP_MS 1250
/* Crude encoding for openedXDisplay */
#define XDPY_DONT_CONNECT 	-1 	/* Don't use X extension */
#define XDPY_NEED_CONNECT 	0	/* Use X extension but not open yet */
#define XDPY_CONNECTED		1	/* Using and opened X extension */

#define MAGIC_OFFSET 3000

extern void packspuCreateFunctions( void );
extern void packspuGatherConfiguration( const SPU *child_spu );
extern void packspuConnectToServer( CRNetServer *server );
extern void packspuFlush( void *arg );
extern void packspuHuge( CROpcode opcode, void *buf );
extern void callbackWindowDelete(unsigned long key, void *data1, void *data2);

extern ThreadInfo *packspuNewThread( CRthread id );
extern void openXDisplay(const char *dpyName);
extern void setUpServerConnection(char *glStub_str);
extern void attemptResume(void);

extern void packspuCreateDiffTable( void );
extern void packspuCreateStateTable( void );

#endif /* CR_PACKSPU_H */
