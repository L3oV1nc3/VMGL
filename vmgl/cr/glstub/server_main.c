/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "server.h"
#include "cr_net.h"
#include "cr_unpack.h"
#include "cr_error.h"
#include "cr_glstate.h"
#include "cr_string.h"
#include "cr_mem.h"
#include "cr_hash.h"
#include <signal.h>
#include <stdlib.h>
#define DEBUG_FP_EXCEPTIONS 0
#if DEBUG_FP_EXCEPTIONS
#include <fpu_control.h>
#include <math.h>
#endif

/**
 * GLStub global data
 */
GLStub cr_server;

int tearingdown = 0; /* can't be static */


/**
 * Return pointer to server's first SPU.
 */
SPU*
glStubHeadSPU(void)
{
	 return cr_server.head_spu;
}



static void DeleteBarrierCallback( void *data )
{
	GLStubBarrier *barrier = (GLStubBarrier *) data;
	crFree(barrier->waiting);
	crFree(barrier);
}


static void deleteContextCallback( void *data )
{
	CRContext *c = (CRContext *) data;
	crStateDestroyContext(c);
}


static void glStubTearDown( void )
{
	GLint i;

	/* avoid a race condition */
	if (tearingdown)
		return;

	tearingdown = 1;

	crStateSetCurrent( NULL );

	cr_server.curClient = NULL;
	cr_server.run_queue = NULL;

	crFree( cr_server.overlap_intens );
	cr_server.overlap_intens = NULL;

	/* Deallocate all semaphores */
	crFreeHashtable(cr_server.semaphores, crFree);
	cr_server.semaphores = NULL;
 
	/* Deallocate all barriers */
	crFreeHashtable(cr_server.barriers, DeleteBarrierCallback);
	cr_server.barriers = NULL;

	/* Free all context info */
	crFreeHashtable(cr_server.contextTable, deleteContextCallback);

	/* Free vertex programs */
	crFreeHashtable(cr_server.programTable, crFree);

	for (i = 0; i < cr_server.numClients; i++) {
		if (cr_server.clients[i]) {
			CRConnection *conn = cr_server.clients[i]->conn;
			crNetFreeConnection(conn);
			crFree(cr_server.clients[i]);
		}
	}
	cr_server.numClients = 0;

#if 1
	/* disable these two lines if trying to get stack traces with valgrind */
	crSPUUnloadChain(cr_server.head_spu);
	cr_server.head_spu = NULL;
#endif

	crUnloadOpenGL();
}


static void
glStubConnCloseCallback( CRConnection *conn )
{
	int i, closingClient = -1, connCount = 0;
	crDebug("GLStub: client connection closed");
	/* count number of connections remaining */
	for (i = 0; i < cr_server.numClients; i++) {
		if (cr_server.clients[i] && cr_server.clients[i]->conn) {
			connCount++;
			if (conn == cr_server.clients[i]->conn) {
				closingClient = i;
			}
		}
	}
	if (closingClient >= 0 && connCount == 1 && cr_server.exitIfNoClients) {
		crWarning("GLStub: Last client disconnected - exiting.");
		exit(0);
	}
}


static void
glStubCleanup( int sigio )
{
	glStubTearDown();

	tearingdown = 0;

	exit(0);
}


void
glStubSetPort(int port)
{
	cr_server.tcpip_port = port;
}



static void
crPrintHelp(void)
{
	printf("Usage: glstub [OPTIONS]\n");
	printf("Options:\n");
	printf("  -port N          Specifies the port number this server will listen to.\n");
	printf("  -v N             Specifies VNC mode, and the XID of the vncviewer's desktop window.\n");
	printf("  -q N             In VNC mode, specifies the port number for incoming X windowing msgs.\n");
	printf("  -help            Prints this information.\n");
}


/**
 * Do GLStub initializations.  After this, we can begin servicing clients.
 */
void
glStubInit(int argc, char *argv[])
{
	int i;
	CRMuralInfo *defaultMural;
	
	cr_server.VNCWin = 0;
	cr_server.secondPort = 0;

	for (i = 1 ; i < argc ; i++)
	{
		if (!crStrncmp( argv[i], "-port", 5)) {
			/* This is the port on which we'll accept client connections */
			if (i == argc - 1)
			{
				crError( "-port requires an argument" );
			}
			cr_server.tcpip_port = crStrToInt(argv[++i]);
		}
		else if (!crStrncmp( argv[i], "-v", 2)) {
			/* This specifies VNC mode and the XID of the vncviewer's desktop window */
			if (i == argc - 1)
			{
				crError( "-v requires an argument" );
			}
			cr_server.VNCWin = (unsigned int) crStrToInt(argv[++i]);
		}
		else if (!crStrncmp( argv[i], "-q", 2)) {
			/* In VNC mode, specifies port for incoming X windowing msgs */
			if (i == argc - 1)
			{
				crError( "-q requires an argument" );
			}
			cr_server.secondPort = (unsigned short) crStrToInt(argv[++i]);
		}
		else if (!crStrcmp( argv[i], "-help" ))	{
			crPrintHelp();
			exit(0);
		}
	}
	
	if (((!cr_server.VNCWin)&&(cr_server.secondPort))
	    ||((cr_server.VNCWin)&&(!cr_server.secondPort)))
		crError( "Need to specify -v AND -q" );
	if (cr_server.secondPort) {
	    XInitThreads();
	    spawnXWindowingThread();
	}

	signal( SIGTERM, glStubCleanup );
	signal( SIGINT, glStubCleanup );
	signal( SIGPIPE, SIG_IGN );

#if DEBUG_FP_EXCEPTIONS
	{
		fpu_control_t mask;
		_FPU_GETCW(mask);
		mask &= ~(_FPU_MASK_IM | _FPU_MASK_DM | _FPU_MASK_ZM
							| _FPU_MASK_OM | _FPU_MASK_UM);
		_FPU_SETCW(mask);
	}
#endif

	cr_server.firstCallCreateContext = GL_TRUE;
	cr_server.firstCallMakeCurrent = GL_TRUE;

	/*
	 * Create default mural info and hash table.
	 */
	cr_server.muralTable = crAllocHashtable();
	defaultMural = (CRMuralInfo *) crCalloc(sizeof(CRMuralInfo));
	crHashtableAdd(cr_server.muralTable, 0, defaultMural);

	cr_server.programTable = crAllocHashtable();

	crNetInit(glStubRecv, glStubConnCloseCallback);
	crStateInit();

	glStubGatherConfiguration();

	crStateLimitsInit( &(cr_server.limits) );

	/*
	 * Default context
	 */
	cr_server.contextTable = crAllocHashtable();
	cr_server.DummyContext = crStateCreateContext( &cr_server.limits,
																								 CR_RGB_BIT | CR_DEPTH_BIT, NULL );
	cr_server.curClient->currentCtx = cr_server.DummyContext;

	glStubInitDispatch();
	crStateDiffAPI( &(cr_server.head_spu->dispatch_table) );

	crUnpackSetReturnPointer( &(cr_server.return_ptr) );
	crUnpackSetWritebackPointer( &(cr_server.writeback_ptr) );

	cr_server.barriers = crAllocHashtable();
	cr_server.semaphores = crAllocHashtable();
}


int main( int argc, char *argv[] )
{
	glStubInit(argc, argv);

	glStubSerializeRemoteStreams();

	glStubTearDown();

	tearingdown = 0;

	return 0;
}
