#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <syslog.h>
#include <signal.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/wait.h>
#include "utils.h"


pthread_mutex_t config_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t ghbn_mutex = PTHREAD_MUTEX_INITIALIZER;
struct in_addr *
wd_gethostbyname(const char *name)
{
	struct hostent *he;
	struct in_addr *h_addr, *in_addr_temp;


	h_addr = malloc(sizeof(struct in_addr));

	LOCK_GHBN();

	he = gethostbyname(name);

	if (he == NULL) {
		free(h_addr);
		UNLOCK_GHBN();
		return NULL;
	}


	in_addr_temp = (struct in_addr *)he->h_addr_list[0];
	h_addr->s_addr = in_addr_temp->s_addr;

	UNLOCK_GHBN();

	return h_addr;
}


int fprintf_locked(FILE *fp, const char *format, ...)
{
    flockfile(fp);
    va_list args;
    va_start(args, format);
    int rc = vfprintf(fp, format, args);
    va_end(args);
    funlockfile(fp);
    return rc;
}
pid_t safe_fork(void) {
	pid_t result;
	result = fork();

	if (result == -1) {
		exit (1);
	}

	return result;
}

int
execute(const char *cmd_line, int quiet)
{
        int pid,
            status,
            rc;

        const char *new_argv[4];
        new_argv[0] = "/bin/sh";
        new_argv[1] = "-c";
        new_argv[2] = cmd_line;
        new_argv[3] = NULL;

        pid = safe_fork();
        if (pid == 0) {    /* for the child process:         */
                if (quiet) close(2);
		if (execvp("/bin/sh", (char *const *)new_argv) == -1) {    /* execute the command  */
    	fprintf_locked(stderr, "execvp() fail %s \n",strerror(errno));    
		} else {
    	fprintf_locked(stderr, "execvp() fail ");    
		}
                exit(1);
        }

	rc = waitpid(pid, &status, 0);

        return (WEXITSTATUS(status));
}

int connect_auth_server() {
	int sockfd;

	LOCK_CONFIG();
	sockfd = _connect_auth_server(0);
	UNLOCK_CONFIG();

	return (sockfd);
}

/* Helper function called by connect_auth_server() to do the actual work including recursion
 * DO NOT CALL DIRECTLY
 @param level recursion level indicator must be 0 when not called by _connect_auth_server()
 */
int _connect_auth_server(int level) {
	struct in_addr *h_addr;
	int num_servers = 0;
	char * hostname = NULL;
	char * popular_servers[] = {
		  "www.baidu.com",
		  "www.sohu.com",
		  NULL
	};
	char ** popularserver;
	char * ip;
	struct sockaddr_in their_addr;
	int sockfd;


	for (popularserver = popular_servers; *popularserver; popularserver++) {
		h_addr = wd_gethostbyname(*popularserver);
		if (h_addr) {
			break;
		}
		else {
		}
	}

	if (!h_addr) {
		return(-1);
	}
	else {
		free(h_addr);
		their_addr.sin_family = AF_INET;
		their_addr.sin_port = htons(PORT);
		memset(&(their_addr.sin_zero), '\0', sizeof(their_addr.sin_zero));
		if (inet_pton(AF_INET, IPSTR, &their_addr.sin_addr) <= 0 ){
			return(-1);
		};

		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
			return(-1);
		}

		if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {

			close(sockfd);
			return(-1);
		}
		else {
			return sockfd;
		}
	}
}


