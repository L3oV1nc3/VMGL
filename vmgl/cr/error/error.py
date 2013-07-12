# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.


import sys

sys.path.append( "../glapi_parser" )
import apiutil

apiutil.CopyrightC()


print """#include <stdio.h>
#include "cr_error.h"
#include "cr_spu.h"
#include "state/cr_statetypes.h"

#define ERROR_APIENTRY

#define ERROR_UNUSED(x) ((void)x)"""


keys = apiutil.GetDispatchedFunctions("../glapi_parser/APIspec.txt")

for func_name in keys:
	return_type = apiutil.ReturnType(func_name)
	params = apiutil.Parameters(func_name)
	print '\nstatic %s ERROR_APIENTRY error%s( %s )' % (return_type, func_name, apiutil.MakeDeclarationString(params ))
	print '{'
	print '\tcrError( "ERROR SPU: Unsupported function gl%s called!" );' % func_name
	# Handle the void parameter list
	for (name, type, vecSize) in params:
		print '\tERROR_UNUSED(%s);' % name

	if return_type != "void":
		print '\treturn (%s)0;' % return_type
	print '}'

print 'SPUNamedFunctionTable _cr_error_table[] = {'
for index in range(len(keys)):
	func_name = keys[index]
	print '\t{ "%s", (SPUGenericFunction) error%s },' % (func_name, func_name )
print '\t{ NULL, NULL }'
print '};'
