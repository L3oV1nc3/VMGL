/*
* Copyright (c) 2006-2007 H. Andres Lagar-Cavilla <andreslc@cs.toronto.edu>
*                                                                                                                                                            * Redistribution and use in source and binary forms, with or without
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
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED                                                                                             * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/

/* Helper daemon to aid suspended OpenGL apps on resumption.
 * vmgl project */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

int sock;
struct sockaddr_un addr;

void sig_handler(int sig) {
    close(sock);
    fprintf(stdout, "SIGINT received\n");
    unlink(addr.sun_path);
    exit(0);
}

int main(int argc, char *argv[]) {
    int sock, len, newSock;
    size_t string_len, int_len = sizeof(size_t);
    char glStub_str[512];
    
    signal(SIGINT, sig_handler);
    
    /* Get string to be sent from command line */
    if (argc < 2) {
	fprintf(stderr, "Usage: %s <string_to_be_sent>\n", argv[0]);
	exit(-1);
    }
    string_len = strlen(argv[1]);
    (void) strncpy(glStub_str, argv[1], string_len);
    glStub_str[string_len] = '\0';    

    /* Set up socket */
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
	fprintf(stderr, "Failed to create socket: %s\n", strerror(errno));
	exit(-1);
    }
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    sprintf(addr.sun_path, "%s/.vmgl_resume", getenv("HOME"));
    len = strlen(addr.sun_path) + sizeof(addr.sun_family);
    if (bind(sock, (struct sockaddr *) &addr, (socklen_t) len)) {
	fprintf(stderr, "Failed to bind socket to %s: %s\n", strerror(errno), addr.sun_path);
	exit(-1);
    }
    if (listen(sock, 1)) {
	fprintf(stderr, "Failed to listen on socket: %s\n", strerror(errno));
	exit(-1);
    }
    
    /* loop forever answering queries */
    while (1) {
        newSock = accept(sock, NULL, (socklen_t *) &len);
        if (newSock == -1) {
	    fprintf(stderr, "Failed to accept connection on socket: %s\n", strerror(errno));
	    exit(-1);
	}
	fprintf(stdout, "New dude connected\n");
	/* Ok, send stuff */
	write(newSock, (void *) &string_len, int_len);
	write(newSock, (void *) glStub_str, string_len);
	close(newSock);
    }
    
    /* Done */
    close(sock);
    exit(0);
}
