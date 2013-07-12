/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_mem.h"
#include "cr_environment.h"
#include "cr_string.h"
#include "cr_dll.h"
#include "cr_error.h"
#include "cr_spu.h"

#include <stdio.h>

#define DLL_SUFFIX ".so"
#define DLL_PREFIX "lib"

extern void __buildDispatch( SPU *spu );

static char *__findDLL( char *name, char *dir )
{
	static char path[8092];
	
	if (!dir)
	{
		sprintf ( path, "%s%sspu%s", DLL_PREFIX, name, DLL_SUFFIX );
	}
	else
	{
		sprintf ( path, "%s/%s%sspu%s", dir, DLL_PREFIX, name, DLL_SUFFIX );
	}
	return path;
}

/**
 *  Load a single SPU from disk and initialize it.  Is there any reason 
 * to export this from the SPU loader library? */

SPU * crSPULoad( SPU *child, int id, char *name, char *dir, void *server )
{
	SPU *the_spu;
	char *path;

	CRASSERT( name != NULL );

	the_spu = (SPU*)crAlloc( sizeof( *the_spu ) );
	the_spu->id = id;
	the_spu->privatePtr = NULL;
	path = __findDLL( name, dir );
	the_spu->dll = crDLLOpen( path, 0/*resolveGlobal*/ );
	the_spu->entry_point = 
		(SPULoadFunction) crDLLGetNoError( the_spu->dll, SPU_ENTRY_POINT_NAME );
	if (!the_spu->entry_point)
	{
		crError( "Couldn't load the SPU entry point \"%s\" from SPU \"%s\"!", 
				SPU_ENTRY_POINT_NAME, name );
	}

	/* This basicall calls the SPU's SPULoad() function */
	if (!the_spu->entry_point( &(the_spu->name), &(the_spu->super_name), 
				   &(the_spu->init), &(the_spu->self), 
				   &(the_spu->cleanup),
				   &(the_spu->options),
				   &(the_spu->spu_flags)) )
	{
		crError( "I found the SPU \"%s\", but loading it failed!", name );
	}
	if (crStrcmp(the_spu->name,"error"))
	{
		/* the default super/base class for an SPU is the error SPU */
		if (the_spu->super_name == NULL)
		{
			the_spu->super_name = "error";
		}
		the_spu->superSPU = crSPULoad( child, id, the_spu->super_name, dir, server );
	}
	else
	{
		the_spu->superSPU = NULL;
	}
	crDebug("Initializing %s SPU", name);
	the_spu->function_table = the_spu->init( id, child, the_spu, 0, 1 );
	__buildDispatch( the_spu );
	/*crDebug( "initializing dispatch table %p (for SPU %s)", (void*)&(the_spu->dispatch_table), name );*/
	crSPUInitDispatchTable( &(the_spu->dispatch_table) );
	/*crDebug( "Done initializing the dispatch table for SPU %s, calling the self function", name );*/

	the_spu->dispatch_table.server = server;
	the_spu->self( &(the_spu->dispatch_table) );
	/*crDebug( "Done with the self function" );*/

	return the_spu;
}

/**
 *  Load the entire chain of SPUs and initialize all of them. 
 * This function returns the first one in the chain.
 */
SPU *
crSPULoadChain( int count, int *ids, char **names, char *dir, void *server )
{
	int i;
	SPU *child_spu = NULL;
	CRASSERT( count > 0 );

	for (i = count-1 ; i >= 0 ; i--)
	{
		int spu_id = ids[i];
		char *spu_name = names[i];
		SPU *the_spu, *temp;
		
		/* This call passes the previous version of spu, which is the SPU's 
		 * "child" in this chain. */

		the_spu = crSPULoad( child_spu, spu_id, spu_name, dir, server );
		if (child_spu != NULL)
		{
			/* keep track of this so that people can pass functions through but 
			 * still get updated when API's change on the fly. */
			for (temp = the_spu ; temp ; temp = temp->superSPU )
			{
				struct _copy_list_node *node = (struct _copy_list_node *) crAlloc( sizeof( *node ) );
				node->copy = &(temp->dispatch_table);
				node->next = child_spu->dispatch_table.copyList;
				child_spu->dispatch_table.copyList = node;
			}
		}
		child_spu = the_spu;
	}
	return child_spu;
}


#if 00
/* XXXX experimental code - not used at this time */
/**
 * Like crSPUChangeInterface(), but don't loop over all functions in
 * the table to search for 'old_func'.
 */
void
crSPUChangeFunction(SPUDispatchTable *table, unsigned int funcOffset,
                    void *newFunc)
{
	SPUGenericFunction *f = (SPUGenericFunction *) table + funcOffset;
	struct _copy_list_node *temp;

	CRASSERT(funcOffset < sizeof(*table) / sizeof(SPUGenericFunction));

	printf("%s\n", __FUNCTION__);
	if (table->mark == 1)
		return;
	table->mark = 1;
	*f = newFunc;

	/* update all copies of this table */
#if 1
	for (temp = table->copyList ; temp ; temp = temp->next)
	{
		crSPUChangeFunction( temp->copy, funcOffset, newFunc );
	}
#endif
	if (table->copy_of != NULL)
	{
		crSPUChangeFunction( table->copy_of, funcOffset, newFunc );
	}
#if 0
	for (temp = table->copyList ; temp ; temp = temp->next)
	{
		crSPUChangeFunction( temp->copy, funcOffset, newFunc );
	}
#endif
	table->mark = 0;
}
#endif



/**
 * Call the cleanup() function for each SPU in a chain, close the SPU
 * DLLs and free the SPU objects.
 * \param headSPU  pointer to the first SPU in the chain
 */
void
crSPUUnloadChain(SPU *headSPU)
{
	SPU *the_spu = headSPU, *next_spu;

	while (the_spu)
	{
		crDebug("Cleaning up SPU %s", the_spu->name);

		if (the_spu->cleanup)
			the_spu->cleanup();

		next_spu = the_spu->superSPU;
#if 0
		/* unfortunately, we sometimes get crashes in glibc when exiting if
		 * we close the SPU library.  So, disable for now.
		 */
		crDLLClose(the_spu->dll);
#endif
		crFree(the_spu);
		the_spu = next_spu;
	}
}
