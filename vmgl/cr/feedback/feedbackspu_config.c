/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_string.h"
#include "cr_environment.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "feedbackspu.h"

#include <stdio.h>

/* option, type, nr, default, min, max, title, callback
 */
SPUOptions feedbackSPUOptions[] = {
    { NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },
};
            
            
void feedbackspuGatherConfiguration( void )
{
	feedback_spu.render_mode = GL_RENDER;
	feedback_spu.default_viewport = 1;
}
