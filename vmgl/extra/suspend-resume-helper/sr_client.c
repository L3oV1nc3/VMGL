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

/* Stupid test helper. Copy & paste of
 * cr/pack/packspu_resume.c
 * vmgl project */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

int main() {
	int sock, len;
	struct sockaddr_un addr;
	char *glStub_str;
	size_t string_len, int_len = sizeof(size_t);
	ssize_t just_read;
	unsigned int pos;
    
	/* Set up the unix domain socket */
	memset(&addr, 0, sizeof(struct sockaddr_un));
        addr.sun_family = AF_UNIX;
	sprintf(addr.sun_path, "%s/.vmgl_resume", getenv("HOME"));
	len = strlen(addr.sun_path) + sizeof(addr.sun_family);

	/* Loop endlessly, this could be a thin client configuration. */
	while(1) {
	    /* Socket calls */
	    sock = socket(AF_UNIX, SOCK_STREAM, 0);
	    if (sock == -1) {
		fprintf(stderr, "Resume: can't create UNIX domain socket: %s\n", strerror(errno));
		exit(-1);
	    }
	    if ((connect(sock, (struct sockaddr *) &addr, len)) == -1) {
		fprintf(stderr, "Resume: connect to UNIX domain socket %s failed: %s\n",
			addr.sun_path, strerror(errno));
		close(sock);
		usleep(750000);
		continue;
	    }

	    /* Connected, get new glStub string */
	    if ((read(sock, (void *) &string_len, int_len)) < (ssize_t) int_len) {
		fprintf(stderr, "Resume: couldn't even read an integer out of UNIX domain socket %s\n",
			addr.sun_path);
		close(sock);
		continue;
	    }
	    glStub_str = (char *) malloc(sizeof(char)*(string_len+1));
	    /* Kinda read exact loop */
	    pos = 0;
	    while (string_len > 0) {
		just_read = read(sock, (void *) &(glStub_str[pos]), string_len);
		if (just_read == -1) {
		    fprintf(stderr, "Resume: error %s while reading from UNIX domain socket %s\n",
			    strerror(errno), addr.sun_path);
		    free(glStub_str);
		    glStub_str = NULL;
		    break;
		}
		string_len -= (size_t) just_read;
		pos += (unsigned int) just_read;
	    }
	    
	    /* Finally, invoke resume code */
	    close(sock);
	    if (glStub_str) {
		glStub_str[pos] = '\0';
		/* glStub_str freed within call below */
		fprintf(stdout, "Resume: resume completed, connected to %s\n", glStub_str);
		break;
	    }    
	}
	/* We're out, means we're good */
	exit(0);
}

