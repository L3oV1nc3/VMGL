/* Copyright (c) 2001, Stanford University
 * All rights reserved.
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#ifndef CR_SERVER_H
#define CR_SERVER_H

#include "cr_protocol.h"
#include "cr_glstate.h"
#include "spu_dispatch_table.h"

#include "state/cr_currentpointers.h"

#include "cr_server.h"

/*
 * This is the base number for window and context IDs
 */
#define MAGIC_OFFSET 5000

extern GLStub cr_server;

/* Semaphore wait queue node */
typedef struct _wqnode {
	RunQueue *q;
	struct _wqnode *next;
} wqnode;

typedef struct {
	GLuint count;
	GLuint num_waiting;
	RunQueue **waiting;
} GLStubBarrier;

typedef struct {
	GLuint count;
	wqnode *waiting, *tail;
} GLStubSemaphore;

typedef struct {
	GLuint id;
	GLint projParamStart;
	GLfloat projMat[16];  /* projection matrix, accumulated via calls to */
                        /* glProgramLocalParameterARB, glProgramParameterNV */
} GLStubProgram;

void glStubGatherConfiguration(void);
void glStubGetTileInfo( CRMuralInfo *mural );
void glStubInitializeTiling(CRMuralInfo *mural);
void glStubInitDispatch(void);
void glStubReturnValue( const void *payload, unsigned int payload_len );
void glStubWriteback(void);
int glStubRecv( CRConnection *conn, CRMessage *msg, unsigned int len );
void glStubSerializeRemoteStreams(void);
void glStubAddToRunQueue( CRClient *client );

void glStubApplyBaseProjection( const CRmatrix *baseProj );
void glStubApplyViewMatrix( const CRmatrix *view );
void glStubSetOutputBounds( const CRMuralInfo *mural, int extNum );
void glStubComputeViewportBounds( const CRViewportState *v, CRMuralInfo *mural );

GLboolean glStubInitializeBucketing(CRMuralInfo *mural);

void glStubNewMuralTiling(CRMuralInfo *mural, GLint muralWidth, GLint muralHeight, GLint numTiles, const GLint *tileBounds);

void crComputeOverlapGeom(double *quads, int nquad, CRPoly ***res);
void crComputeKnockoutGeom(double *quads, int nquad, int my_quad_idx, CRPoly **res);

int glStubGetCurrentEye(void);

GLint glStubSPUWindowID(GLint serverWindow);

void spawnXWindowingThread(void);

#endif /* CR_SERVER_H */
