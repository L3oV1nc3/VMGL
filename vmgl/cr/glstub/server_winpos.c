
#include "server_dispatch.h"
#include "server.h"


/**
 * All glWindowPos commands go through here.
 * The (x,y,z) coordinate is in mural-space window coords.
 * We need to bias by the current tile's position.
 * Remember that the state differencer (used in the tilesort SPU) uses
 * glWindowPos to update the raster position on the servers, so that takes
 * care of proper position for tilesorting.
 * Also, this helps with sort-last rendering: if images are being sent to a
 * server that has tiles, the images will be correctly positioned too.
 * This also solves Eric Mueller's "tilesort-->readback trouble" issue.
 *
 * glRasterPos commands will go through unmodified; they're transformed
 * by the modelview/projection matrix which is already modified per-tile.
 */
static void glStubWindowPos( GLfloat x, GLfloat y, GLfloat z )
{
	CRMuralInfo *mural = cr_server.curClient->currentMural;
	x -= (float) mural->extents[mural->curExtent].imagewindow.x1;
	y -= (float) mural->extents[mural->curExtent].imagewindow.y1;
	crStateWindowPos3fARB(x, y, z);
	cr_server.head_spu->dispatch_table.WindowPos3fARB(x, y, z);
}


void SERVER_DISPATCH_APIENTRY glStubDispatchWindowPos2dARB( GLdouble x, GLdouble y )
{
	glStubWindowPos((GLfloat) x, (GLfloat) y, 0.0F);
}

void SERVER_DISPATCH_APIENTRY glStubDispatchWindowPos2dvARB( const GLdouble * v )
{
	glStubWindowPos((GLfloat) v[0], (GLfloat) v[1], 0.0F);
}

void SERVER_DISPATCH_APIENTRY glStubDispatchWindowPos2fARB( GLfloat x, GLfloat y )
{
	glStubWindowPos(x, y, 0.0F);
}

void SERVER_DISPATCH_APIENTRY glStubDispatchWindowPos2fvARB( const GLfloat * v )
{
	glStubWindowPos(v[0], v[1], 0.0F);
}

void SERVER_DISPATCH_APIENTRY glStubDispatchWindowPos2iARB( GLint x, GLint y )
{
	glStubWindowPos((GLfloat)x, (GLfloat)y, 0.0F);
}

void SERVER_DISPATCH_APIENTRY glStubDispatchWindowPos2ivARB( const GLint * v )
{
	glStubWindowPos((GLfloat)v[0], (GLfloat)v[1], 0.0F);
}

void SERVER_DISPATCH_APIENTRY glStubDispatchWindowPos2sARB( GLshort x, GLshort y )
{
	glStubWindowPos(x, y, 0.0F);
}

void SERVER_DISPATCH_APIENTRY glStubDispatchWindowPos2svARB( const GLshort * v )
{
	glStubWindowPos(v[0], v[1], 0.0F);
}

void SERVER_DISPATCH_APIENTRY glStubDispatchWindowPos3dARB( GLdouble x, GLdouble y, GLdouble z )
{
	glStubWindowPos((GLfloat) x, (GLfloat) y, (GLfloat) z);
}

void SERVER_DISPATCH_APIENTRY glStubDispatchWindowPos3dvARB( const GLdouble * v )
{
	glStubWindowPos((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2]);
}

void SERVER_DISPATCH_APIENTRY glStubDispatchWindowPos3fARB( GLfloat x, GLfloat y, GLfloat z )
{
	glStubWindowPos(x, y, z);
}

void SERVER_DISPATCH_APIENTRY glStubDispatchWindowPos3fvARB( const GLfloat * v )
{
	glStubWindowPos(v[0], v[1], v[2]);
}

void SERVER_DISPATCH_APIENTRY glStubDispatchWindowPos3iARB( GLint x, GLint y, GLint z )
{
	glStubWindowPos((GLfloat)x,(GLfloat)y, (GLfloat)z);
}

void SERVER_DISPATCH_APIENTRY glStubDispatchWindowPos3ivARB( const GLint * v )
{
	glStubWindowPos((GLfloat)v[0], (GLfloat)v[1], (GLfloat)v[2]);
}

void SERVER_DISPATCH_APIENTRY glStubDispatchWindowPos3sARB( GLshort x, GLshort y, GLshort z )
{
	glStubWindowPos(x, y, z);
}

void SERVER_DISPATCH_APIENTRY glStubDispatchWindowPos3svARB( const GLshort * v )
{
	glStubWindowPos(v[0], v[1], v[2]);
}

