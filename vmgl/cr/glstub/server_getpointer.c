/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "chromium.h"
#include "cr_error.h" 
#include "server_dispatch.h"
#include "server.h"

void SERVER_DISPATCH_APIENTRY glStubDispatchGetPointerv( GLenum pname, GLvoid **pointer )
{
	crError( "glGetPointerv isn't *ever* allowed to be on the wire!" );
	(void) pname;
	(void) pointer;
}

void SERVER_DISPATCH_APIENTRY glStubDispatchGetVertexAttribPointervNV( GLuint index, GLenum pname, GLvoid ** pointer )
{
	crError( "glGetVertexAttribPointervNV isn't *ever* allowed to be on the wire!" );
	(void) pname;
	(void) pointer;
}

void SERVER_DISPATCH_APIENTRY glStubDispatchGetVertexAttribPointervARB( GLuint index, GLenum pname, GLvoid ** pointer )
{
	crError( "glGetVertexAttribPointervNV isn't *ever* allowed to be on the wire!" );
	(void) pname;
	(void) pointer;
}

