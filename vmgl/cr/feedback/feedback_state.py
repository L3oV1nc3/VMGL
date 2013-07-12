# Copyright (c) 2001, Stanford University
# All rights reserved.
#
# See the file LICENSE.txt for information on redistributing this software.

import sys

sys.path.append( "../glapi_parser" )
import apiutil


apiutil.CopyrightC()

print """
#include "cr_server.h"
#include "feedbackspu.h"
#include "feedbackspu_proto.h"
"""

keys = apiutil.GetDispatchedFunctions("../glapi_parser/APIspec.txt")

for func_name in keys:
	if apiutil.FindSpecial( "feedback_state", func_name ):
		return_type = apiutil.ReturnType(func_name)
		params = apiutil.Parameters(func_name)
		print '%s FEEDBACKSPU_APIENTRY feedbackspu_%s( %s )' % (return_type, func_name, apiutil.MakeDeclarationString(params))
		print '{'
		print '\tcrState%s( %s );' % (func_name, apiutil.MakeCallString(params))
		print ''
		print '\tfeedback_spu.super.%s( %s );' % (func_name, apiutil.MakeCallString(params))
		print '}'
