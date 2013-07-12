/* Copyright (c) 2001, Stanford University
 * All rights reserved
 *
 * See the file LICENSE.txt for information on redistributing this software.
 */

#include "packspu.h"
#include "cr_string.h"
#include "cr_error.h"
#include "cr_spu.h"
#include "cr_mem.h"
#include "cr_environment.h"

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

static void __setDefaults( void )
{
	crMemZero(pack_spu.context, CR_MAX_CONTEXTS * sizeof(ContextInfo));
	pack_spu.numContexts = 0;

	crMemZero(pack_spu.thread, MAX_THREADS * sizeof(ThreadInfo));
	pack_spu.numThreads = 0;
}


/* SPU options commented out for the time being. Andres */
//SPUOptions packSPUOptions[] = { 
//	{ "emit_GATHER_POST_SWAPBUFFERS", CR_BOOL, 1, "0", NULL, NULL, 
//	  "Emit a parameteri after SwapBuffers", (SPUOptionCB)set_emit },
//
//	{ "swapbuffer_sync", CR_BOOL, 1, "1", NULL, NULL,
//		"Sync on SwapBuffers", (SPUOptionCB) set_swapbuffer_sync },
//
//	{ NULL, CR_BOOL, 0, NULL, NULL, NULL, NULL, NULL },
//};

void setUpServerConnection(char *glStub_str)
{
    char *glStubPort_str, *newPort_str;
    unsigned short glStubPort, buf[2];
    int sock;
    struct sockaddr_in serverAddress;
    struct hostent *glStub = NULL;
    size_t bufLen = sizeof(short) * 2;

    /* Parse server string. */
    glStubPort_str = crStrchr(glStub_str,':');
    if (!glStubPort_str)
	crError("GLSTUB format must be \"hostname:port\"");
    *glStubPort_str++ = 0;
    glStub = gethostbyname(glStub_str);
    if (!glStub) 
	crError("Hostname %s unknown",glStub_str);
    glStubPort = (unsigned short) strtoul(glStubPort_str, (char **) NULL, 10);
	
    /* Set up TCP connection */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    crMemset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    crMemcpy((char *) &serverAddress.sin_addr, glStub->h_addr, sizeof(serverAddress.sin_addr));
    serverAddress.sin_port = htons(glStubPort);
    if (connect(sock, (struct sockaddr *) &serverAddress, sizeof(serverAddress)))
	crError("Failed connect to %s:%d",glStub_str,glStubPort);
	    
    /* Talk just a little bit */
    if ((read(sock, (void *) buf, bufLen)) != ((ssize_t) bufLen))
	crError("Failed to read setup msg from %s:%d",glStub_str,glStubPort);
	    
    /* Set up global vars */
    newPort_str = (char *) crAlloc(sizeof(char)*7);
    newPort_str[0] = ':';
    crUShortToString(ntohs(buf[0]),&(newPort_str[1]),10);
    pack_spu.name = crStrjoin3("tcpip://", glStub_str, newPort_str);
    pack_spu.serverIP = ntohl(serverAddress.sin_addr.s_addr);
    pack_spu.secondPort = ntohs(buf[1]);
    if (pack_spu.secondPort)
	pack_spu.openedXDisplay = XDPY_NEED_CONNECT;

    /* Done */
    close(sock);
    glStub_str[strlen(glStub_str)] = ':';
    crFree(glStub_str);
    crFree(newPort_str);

}

void packspuGatherConfiguration( const SPU *child_spu )
{
	char *glStub_str;
	
	__setDefaults();
	pack_spu.emit_GATHER_POST_SWAPBUFFERS= 0;
	pack_spu.swapbuffer_sync = 1;

	pack_spu.buffer_size = 262144; //crMothershipGetMTU( conn );

	glStub_str = crStrdup(crGetenv("GLSTUB"));
	if (!glStub_str) 
	    crError("Need to know glStub environment variable");
	setUpServerConnection(glStub_str);
}
