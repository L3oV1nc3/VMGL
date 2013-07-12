/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_spu.h"
#include "cr_error.h"
#include "cr_string.h"
#include <stdio.h>

/**
 * \mainpage spu_loader 
 *
 * \section Spu_loaderIntroduction Introduction
 *
 * Chromium consists of all the top-level files in the cr
 * directory.  The spu_loader module basically takes care of API dispatch,
 * and OpenGL state management.
 *
 */
void crSPUInitDispatchTable( SPUDispatchTable *table )
{
	table->copyList = NULL;
	table->copy_of = NULL;
	table->mark = 0;
	table->server = NULL;
}

/** Use the default values for all the options:
 */
void crSPUSetDefaultParams( void *spu, SPUOptions *options )
{
	int i;
	
	for (i = 0 ; options[i].option ; i++)
	{
		SPUOptions *opt = &options[i];
		opt->cb( spu, opt->deflt );
	}
}


/**
 * Find the index of the given enum value in the SPUOption's list of
 * possible enum values.
 * Return the enum index, or -1 if not found.
 */
int crSPUGetEnumIndex( const SPUOptions *options, const char *optName, const char *value )
{
	const SPUOptions *opt;
	const int valueLen = crStrlen(value);

	/* first, find the right option */
	for (opt = options; opt->option; opt++) {
		if (crStrcmp(opt->option, optName) == 0) {
			char **values;
			int i;

			CRASSERT(opt->type == CR_ENUM);

			/* break into array of strings */
			/* min string should be of form "'enum1', 'enum2', 'enum3', etc" */
			values = crStrSplit(opt->min, ",");

			/* search the array */
			for (i = 0; values[i]; i++) {
				/* find leading quote */
				const char *e = crStrchr(values[i], '\'');
				CRASSERT(e);
				if (e) {
					/* test for match */
					if (crStrncmp(value, e + 1, valueLen) == 0 &&	e[valueLen + 1] == '\'') {
						crFreeStrings(values);
						return i;
					}
				}
			}

			/* enum value not found! */
			crFreeStrings(values);
			return -1;
		}
	}

	return -1;
}
