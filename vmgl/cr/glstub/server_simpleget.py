# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

import sys

sys.path.append( "../glapi_parser" )
import apiutil


apiutil.CopyrightC()

print """#include "cr_spu.h"
#include "chromium.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "cr_net.h"
#include "server_dispatch.h"
#include "server.h"
"""

from get_sizes import *;


funcs = [ 'GetIntegerv', 'GetFloatv', 'GetDoublev', 'GetBooleanv' ]
types = [ 'GLint', 'GLfloat', 'GLdouble', 'GLboolean' ]

for index in range(len(funcs)):
	func_name = funcs[index]
	params = apiutil.Parameters(func_name)
	print 'void SERVER_DISPATCH_APIENTRY glStubDispatch%s( %s )' % ( func_name, apiutil.MakeDeclarationString(params))
	print '{'
	print '\t%s *get_values;' % types[index]
	print '\tint tablesize = __numValues( pname ) * sizeof(%s);' % types[index]
	print '\t(void) params;'
	print '\tget_values = (%s *) crAlloc( tablesize );' % types[index]
	print '\tcr_server.head_spu->dispatch_table.%s( pname, get_values );' % func_name
	print '\tglStubReturnValue( get_values, tablesize );'
	print '\tcrFree(get_values);'
	print '}\n'
