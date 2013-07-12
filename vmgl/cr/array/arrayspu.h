/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef ARRAY_SPU_H
#define ARRAY_SPU_H

#define ARRAYSPU_APIENTRY

#include "cr_spu.h"
#include "cr_glstate.h"

void arrayspuGatherConfiguration( void );

typedef struct {
	int id;
	int has_child;
	CRContext *ctx;
	SPUDispatchTable self, child, super;
} ArraySPU;

extern ArraySPU array_spu;

#endif /* ARRAY_SPU_H */
