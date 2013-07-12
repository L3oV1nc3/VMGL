/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "arrayspu.h"
#include <stdio.h>

extern SPUNamedFunctionTable _cr_array_table[];

extern SPUOptions arraySPUOptions[];

static SPUFunctions array_functions = {
	NULL, /* CHILD COPY */
	NULL, /* DATA */
	_cr_array_table /* THE ACTUAL FUNCTIONS */
};

static SPUFunctions *arraySPUInit( int id, SPU *child, SPU *self,
		unsigned int context_id,
		unsigned int num_contexts )
{

	(void) context_id;
	(void) num_contexts;

	array_spu.id = id;
	array_spu.has_child = 0;
	if (child)
	{
		crSPUInitDispatchTable( &(array_spu.child) );
		crSPUCopyDispatchTable( &(array_spu.child), &(child->dispatch_table) );
		array_spu.has_child = 1;
	}
	crSPUInitDispatchTable( &(array_spu.super) );
	crSPUCopyDispatchTable( &(array_spu.super), &(self->superSPU->dispatch_table) );
	arrayspuGatherConfiguration();

	crStateInit();
	array_spu.ctx = crStateCreateContext( NULL, 0, NULL );
#ifdef CR_ARB_vertex_buffer_object
	array_spu.ctx->bufferobject.retainBufferData = GL_TRUE;
#endif
	/* we call SetCurrent instead of MakeCurrent as the differencer
	 * isn't setup yet anyway */
	crStateSetCurrent( array_spu.ctx );

	return &array_functions;
}

static void arraySPUSelfDispatch(SPUDispatchTable *self)
{
	crSPUInitDispatchTable( &(array_spu.self) );
	crSPUCopyDispatchTable( &(array_spu.self), self );
}

static int arraySPUCleanup(void)
{
	return 1;
}

int SPULoad( char **name, char **super, SPUInitFuncPtr *init,
	     SPUSelfDispatchFuncPtr *self, SPUCleanupFuncPtr *cleanup,
	     SPUOptionsPtr *options, int *flags )
{
	*name = "array";
	*super = "passthrough";
	*init = arraySPUInit;
	*self = arraySPUSelfDispatch;
	*cleanup = arraySPUCleanup;
	*options = arraySPUOptions;
	*flags = (SPU_NO_PACKER|SPU_NOT_TERMINAL|SPU_MAX_SERVERS_ZERO);
	
	return 1;
}
