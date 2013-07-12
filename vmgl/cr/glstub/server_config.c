/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "cr_mem.h"
#include "cr_environment.h"
#include "cr_string.h"
#include "cr_error.h"
#include "cr_glstate.h"
#include "server.h"

static void
setDefaults(void)
{
	if (!cr_server.tcpip_port)
		cr_server.tcpip_port = DEFAULT_SERVER_PORT;
	cr_server.run_queue = NULL;
	cr_server.optimizeBucket = 1;
	cr_server.useL2 = 0;
	cr_server.maxBarrierCount = 0;
	cr_server.ignore_papi = 0;
	cr_server.only_swap_once = 0;
	cr_server.overlapBlending = 0;
	cr_server.debug_barriers = 0;
	cr_server.sharedDisplayLists = 1;
	cr_server.sharedTextureObjects = 1;
	cr_server.sharedPrograms = 1;
	cr_server.sharedWindows = 0;
	cr_server.useDMX = 0;
	cr_server.exitIfNoClients = 1;
	cr_server.vpProjectionMatrixParameter = -1;
	cr_server.vpProjectionMatrixVariable = NULL;
	cr_server.currentProgram = 0;

	cr_server.num_overlap_intens = 0;
	cr_server.overlap_intens = 0;
	cr_server.SpuContext = 0;

	crMatrixInit(&cr_server.viewMatrix[0]);
	crMatrixInit(&cr_server.viewMatrix[1]);
	crMatrixInit(&cr_server.projectionMatrix[0]);
	crMatrixInit(&cr_server.projectionMatrix[1]);
	cr_server.currentEye = -1;
}

void
glStubGatherConfiguration()
{
	/* Not really used */
//	char response[8096];
//	char hostname[1024];

	int spu_ids;
	char *spu_names[2];

	CRMuralInfo *defaultMural = (CRMuralInfo *) crHashtableSearch(cr_server.muralTable, 0);
	CRASSERT(defaultMural);

	setDefaults();

	spu_ids = 0;
	spu_names[0] = crStrdup("render");
	spu_names[1] = NULL;

	/*
	 * Load the SPUs
	 */
	cr_server.head_spu =
		crSPULoadChain(1, &spu_ids, spu_names, NULL, &cr_server);
			  /* num_spus */          /* spu_dir */

	/* Need to do this as early as possible */
	cr_server.head_spu->dispatch_table.GetIntegerv(GL_VIEWPORT, (GLint *) defaultMural->underlyingDisplay);
	cr_server.head_spu->dispatch_table.ChromiumParametervCR(GL_VNCWIN_CR, GL_INT, sizeof(cr_server.VNCWin), &cr_server.VNCWin);
                
	crFree(spu_names[0]);

	cr_server.mtu = 262144; //crMothershipGetMTU( conn );
	cr_server.numClients = 1;


	/*
	 * Connect to initial set of clients.
	 * Call crNetAcceptClient() for each client.
	 */
	CRClient *newClient = (CRClient *) crCalloc(sizeof(CRClient));
	newClient->spu_id = 1;
	crStrcpy(cr_server.protocol,"tcpip"); 
	newClient->conn = crNetAcceptClient(cr_server.protocol, NULL, cr_server.tcpip_port, cr_server.mtu);
	newClient->currentCtx = cr_server.DummyContext;
	glStubAddToRunQueue(newClient);
	cr_server.clients[0] = newClient;
	cr_server.curClient = cr_server.clients[0];
	cr_server.curClient->currentMural = defaultMural;
	cr_server.client_spu_id = cr_server.clients[0]->spu_id;

	glStubGetTileInfo(defaultMural);

}
