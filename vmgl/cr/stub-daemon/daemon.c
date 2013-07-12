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

/* VMGL project: stand alone vmgl stub daemon */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>

#include "libvmglstubdaemon/vmgl-stub-daemon.h"

#define DEFAULT_PORT 7000

static unsigned short daemonPort;
static unsigned int ViewerWindow = 0;

static void exitHandler(int sig) {
    fprintf(logfd, "Exiting...\n\n");
    fclose(logfd);
    exit(0);
}

static void daemonize() {
    pid_t pid;
    int tmp;
    
//    chdir(getenv("HOME"));
//    if ((error_fd = fopen(".stub-daemon.log", "a")) < 0) 
//	error_fd = NULL;
//    else
//	setbuf(error_fd,(char *) NULL);
    
    if ((pid = fork()) < 0)
	fail("couldn't fork on daemonize");
    if (pid)
	exit(0);
    setsid();
    close(0);
    close(1);
    close(2);
    tmp = open("/dev/null",O_RDWR);
    dup(tmp);
    dup(tmp);
//    signal(SIGCHLD, reaper);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTERM, exitHandler);
    signal(SIGINT, exitHandler);
}

static void parseArgs(int argc, char *argv[]) {
    int i;
    
    daemonPort = DEFAULT_PORT;
    for (i = 1 ; i < argc ; i++) {
	if ((!strncmp( argv[i], "-port", 5 ))||(!strncmp( argv[i], "-p",2 ))) {
	    if (i == argc - 1) 
        	fail( "-port requires an argument" );
	    daemonPort = strtoul(argv[++i], (char **) NULL, 10);
	} else if ((!strncmp( argv[i], "-viewerwin", 7))||(!strncmp( argv[i], "-v", 2))) {
	    if (i == argc - 1) 
        	fail( "-viewerwin requires an argument" );
    	    ViewerWindow = strtoul(argv[++i], (char **) NULL, 10);
	} else if ((!strncmp( argv[i], "-help", 5))||(!strncmp( argv[i], "-h", 2))) {
	    /* Usage */
	    printf("\t%s [-p/-port <port>] [-v/-viewerin <Viewer Window ID>] [-help/-h]\n", argv[0]);
	    printf("\t<port>: port on which to listen for incoming vmgl lib requests. Default %d\n", DEFAULT_PORT);
	    printf("\t<viewerwin>: XID of the Viewer's desktop window.\n");
	    printf("\t\tPresence implies Viewer mode, absence implies X forwarding mode\n");
	    printf("\t<help>: this help\n");
            exit(0);
        }
    }
}

int main(int argc, char *argv[]) {

    parseArgs(argc,argv);
    
    daemonize();
    
//    acceptLoop(listenOnPort(daemonPort));

    pthread_join(daemonStart(ViewerWindow, daemonPort, ".stub-daemon.log"), NULL);

    return 0;
}
