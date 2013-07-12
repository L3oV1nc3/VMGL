/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_mem.h"
#include "cr_spu.h"
#include "cr_glstate.h"
#include "packspu.h"
#include "cr_packfunctions.h"
#include <stdio.h>
#include <signal.h>

extern SPUNamedFunctionTable _cr_pack_table[];

SPUFunctions pack_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	_cr_pack_table /* THE ACTUAL FUNCTIONS */
};

PackSPU pack_spu;

#ifdef CHROMIUM_THREADSAFE
CRtsd _PackTSD;
CRmutex _PackMutex;
#endif

static void sigusr2(int sig) {
    pack_spu.doResume = 1;
}

static SPUFunctions *
packSPUInit( int id, SPU *child, SPU *self,
						 unsigned int context_id,
						 unsigned int num_contexts )
{
	ThreadInfo *thread;

	(void) context_id;
	(void) num_contexts;
	(void) child;
	(void) self;

#ifdef CHROMIUM_THREADSAFE
	crInitTSD(&_PackTSD);
	crInitMutex(&_PackMutex);
#endif

	pack_spu.id = id;

	/* VMGL: */
	pack_spu.XWindows = crAllocHashtable();
	pack_spu.XDisplay = NULL;
	pack_spu.openedXDisplay = XDPY_DONT_CONNECT;
	pack_spu.contextList = crAllocList();
	pack_spu.doResume = 0;
	signal(SIGUSR2, sigusr2);

	packspuGatherConfiguration( child );
	
	/* This connects to the server, sets up the packer, etc. */
	thread = packspuNewThread( crThreadID() );
	CRASSERT( thread == &(pack_spu.thread[0]) );

	packspuCreateFunctions();
	crStateInit();
	/* For state resume */
	packspuCreateDiffTable();
	packspuCreateStateTable();

	return &pack_functions;
}

static void
packSPUSelfDispatch(SPUDispatchTable *self)
{
	(void)self;
}
                                
static int
packSPUCleanup(void)
{
	/* Should clean up contexts as well? */
	crHashtableWalk(pack_spu.XWindows,callbackWindowDelete,NULL);
	return 1;
}

//extern SPUOptions packSPUOptions[];

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options, int *flags )
{
	*name = "pack";
	*super = NULL;
	*init = packSPUInit;
	*self = packSPUSelfDispatch;
	*cleanup = packSPUCleanup;
	*options = NULL; //packSPUOptions;
	*flags = (SPU_HAS_PACKER|SPU_IS_TERMINAL|SPU_MAX_SERVERS_ONE);

	return 1;
}
