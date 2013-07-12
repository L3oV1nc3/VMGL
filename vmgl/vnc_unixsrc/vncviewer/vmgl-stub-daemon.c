/*
* Copyright (c) 2006-2007 H. Andres Lagar-Cavilla <andreslc@cs.toronto.edu>
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*
*   1. Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
*   2. Redistributions in binary form must reproduce the above
* copyright notice, this list of conditions and the following disclaimer
* in the documentation and/or other materials provided with the
* distribution.
*   3. The name of the author may not be used to endorse or promote
* products derived from this software without specific prior written
* permission.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/
/* VMGL project: daemon to spawn gl stubs. */

#if defined(FreeBSD)
#include <netinet/in.h>
#include <sys/types.h>
#endif
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "vmgl-stub-daemon.h"

#define DEFAULT_END 65535

static unsigned short daemonPort;
static unsigned int ViewerWindow = 0;
static int srvSock;
FILE *logfd;

static void reaper(int sig) {
    waitpid(-1, NULL, WNOHANG);
}

void fail(const char *format, ... ) {
    va_list args;
    int errcode = errno;

    fprintf(logfd, "Error -> ");
    va_start(args, format);
    vfprintf(logfd, format, args);
    va_end(args);
    fprintf(logfd, ": %s\n", strerror(errcode));
    if (logfd != stderr)
	fclose(logfd);
    exit(1);
}

static int listenOnPort(unsigned short port) {
    int sock;
    struct sockaddr_in addr;
    void *dummy;
    size_t lenAddr = sizeof (struct sockaddr_in);
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
	fail("create socket");
    dummy = memset((void *) &addr, 0, lenAddr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(sock, (struct sockaddr *) &addr, lenAddr)) 
	fail("bind socket to port %hu", port);
    if (listen(sock,64))
	fail("listen on socket");
    return sock;
}

static unsigned short findPort(unsigned short base, unsigned short end) {
    int sock;
    struct sockaddr_in addr;
    unsigned short port;
    size_t lenAddr = sizeof(struct sockaddr_in);
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
	fail("create socket");
    memset((void *) &addr, 0, lenAddr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    
    for (port=base;port<=end;port++) {
	addr.sin_port = htons(port);
	if (bind(sock, (struct sockaddr *) &addr, lenAddr)) {
	    if (errno == EADDRINUSE) 
		/* This port is in use, continue */
    		continue;
	    else 
		fail("bind socket to port %hu",port);
	} else {
	    /* Bind succeeded, this port is open */
	    if (close(sock))
    		fail("close on socket");
    	    return port;
	}
    }
    return 0;
}

static void *acceptLoop(void *blah) {
    unsigned int tmp;
    int sock;
    char *glStubArgs[8];
    unsigned short firstPort, secondPort, writeBuf[2];
    pid_t pid;
    size_t writeLen = sizeof(unsigned short) * 2;
    
    signal(SIGCHLD, reaper);
    glStubArgs[0] = "glstub";
    glStubArgs[1] = "-port";
    if (ViewerWindow) {
	glStubArgs[3] = "-v";
	glStubArgs[4] = (char *) malloc(sizeof(char)*32);
	sprintf(glStubArgs[4], "%u", ViewerWindow);
	glStubArgs[5] = "-q";
	glStubArgs[7] = (char *) NULL;
    } else
	glStubArgs[3]= (char *) NULL;
    
    for(;;) {
	if ((sock = accept(srvSock, (struct sockaddr *) NULL, &tmp)) <= 0)
	    fail("accept on socket");
	
	if (!(firstPort = findPort(daemonPort+1,DEFAULT_END))) {
	    close(sock);
	    fail("couldn't find any available open ports");
	}
	writeBuf[0] = htons(firstPort);
	glStubArgs[2] = (char *) malloc(sizeof(char)*8);
	sprintf(glStubArgs[2], "%d", firstPort);
	
	if (ViewerWindow) {
	    if (!(secondPort = findPort(firstPort+1,DEFAULT_END))) {
		close(sock);
		fail("couldn't find any available open ports");
	    }
	    writeBuf[1] = htons(secondPort);
	    glStubArgs[6] = (char *) malloc(sizeof(char)*8);
	    sprintf(glStubArgs[6], "%d", secondPort);
	}
	
	/* Found all ports, glStubArgs and writeBuf are ready. First fork, then send */
	if ((pid = fork()) < 0) {
	    /* Fork failed */
	    close(sock);
	    fail("couldn't fork");
	}
	if (!pid) { /* Fork succesful, child */
	    execvp("glstub", glStubArgs);
	    /* We're still executing here, means fork failed */
	    fail("exec for child failed");
	}
	/* Fork succesful, parent: send writeBuf */
	usleep(1000*250);
	if (write(sock, (void *) writeBuf, writeLen) != (ssize_t) writeLen) {
	    /* Failure, kill the newborn */
	    kill(pid, SIGINT);
	    close(sock);
	    fail("write on socket");
	}
	
	/* All done, finish */
	free(glStubArgs[2]);
	if (ViewerWindow)
	    free(glStubArgs[6]);
	if (close(sock))
	    fail("close on socket");
    }

    /* Should never get here */
    if (close(srvSock))
	fail("close on socket");
    return blah;
}

static void initLog(char *logName) {
    char logFileName[1024];
    char *homeDir = NULL;

    logfd = stderr;
    if (!logName)
	return;
    homeDir = getenv("HOME");
    if (!homeDir)
	return;
    sprintf(logFileName, "%s/%s", homeDir, logName);
    logfd = fopen(logFileName, "a+");
    if (!logfd) {
	logfd = stderr;
	return;
    }
    setbuf(logfd, NULL);    
}

pthread_t daemonStart(unsigned int window, unsigned short basePort, char *logName) {
    pthread_t thread;
    
    ViewerWindow = window;
    initLog(logName);
    daemonPort = findPort(basePort,DEFAULT_END);
    srvSock = listenOnPort(daemonPort);
    fprintf(logfd, "\nSet GLSTUB var in guest to point to port %hu\n", daemonPort);
    if (pthread_create(&thread,NULL,acceptLoop,NULL))
	fail("couldn't create thread");

    return thread;
}
