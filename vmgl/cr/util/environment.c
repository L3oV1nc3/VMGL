/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_environment.h"
#include "cr_string.h"
#include "cr_mem.h"
#include <stdlib.h>
#include <stdio.h>

void crSetenv( const char *var, const char *value )
{
#if defined(LINUX) || defined(FreeBSD)
	setenv( var, value, 1 /* replace */ );
#else
        unsigned long len;
        char *buf;
		
        len = crStrlen(var) + 1 + crStrlen(value) + 1;
        buf = (char *) crAlloc( len );
        sprintf( buf, "%s=%s", var, value );
        putenv( buf );
#endif
}

const char *crGetenv( const char *var )
{
	return getenv( var );
}
