/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef FEEDBACK_SPU_H
#define FEEDBACK_SPU_H

#define FEEDBACKSPU_APIENTRY

#include "cr_spu.h"
#include "cr_timer.h"
#include "cr_glstate.h"

typedef struct {
	int id;
	int has_child;
	SPUDispatchTable self, child, super;

	int render_mode;

	int default_viewport;

	CRCurrentStatePointers current;
} feedbackSPU;

extern feedbackSPU feedback_spu;

extern SPUNamedFunctionTable _cr_feedback_table[];

extern SPUOptions feedbackSPUOptions[];

extern void feedbackspuGatherConfiguration( void );

#endif /* FEEDBACK_SPU_H */
