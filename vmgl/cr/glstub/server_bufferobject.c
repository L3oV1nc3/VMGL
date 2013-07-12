/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "chromium.h"
#include "cr_error.h"
#include "cr_mem.h"
#include "server_dispatch.h"
#include "server.h"

void * SERVER_DISPATCH_APIENTRY
glStubDispatchMapBufferARB( GLenum target, GLenum access )
{
	return NULL;
}

GLboolean SERVER_DISPATCH_APIENTRY
glStubDispatchUnmapBufferARB( GLenum target )
{
	return GL_FALSE;
}

void SERVER_DISPATCH_APIENTRY
glStubDispatchGenBuffersARB(GLsizei n, GLuint *buffers)
{
	GLuint *local_buffers = (GLuint *) crAlloc( n * sizeof(*local_buffers) );
	(void) buffers;
	cr_server.head_spu->dispatch_table.GenBuffersARB( n, local_buffers );
	glStubReturnValue( local_buffers, n * sizeof(*local_buffers) );
	crFree( local_buffers );
}

void SERVER_DISPATCH_APIENTRY
glStubDispatchGetBufferPointervARB(GLenum target, GLenum pname, GLvoid **params)
{
	crError( "glGetBufferPointervARB isn't *ever* allowed to be on the wire!" );
	(void) target;
	(void) pname;
	(void) params;
}

void SERVER_DISPATCH_APIENTRY
glStubDispatchGetBufferSubDataARB(GLenum target, GLintptrARB offset,
																		GLsizeiptrARB size, void * data)
{
	void *b;

	b = crAlloc(size);
	if (b) {
		cr_server.head_spu->dispatch_table.GetBufferSubDataARB( target, offset, size, b );

		glStubReturnValue( b, size );
		crFree( b );
	}
	else {
		crError("Out of memory in glStubDispatchGetBufferSubDataARB");
	}
}

