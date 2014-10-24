#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <string.h>

#include <sys/wait.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <net/if.h>
#include "fetch_thread.h"


static pthread_t tid_fetch = 0;

void
sigchld_handler(int s)
{
	int	status;
	pid_t rc;
	

	rc = waitpid(-1, &status, WNOHANG);

}

termination_handler(int s)
{
	static	pthread_mutex_t	sigterm_mutex = PTHREAD_MUTEX_INITIALIZER;

	if (pthread_mutex_trylock(&sigterm_mutex)) {
		
		pthread_exit(NULL);
	}
	
	if (tid_fetch) {
		
		pthread_kill(tid_fetch, SIGKILL);
	}


	exit(s == 0 ? 1 : 0);
}


static void
init_signals(void)
{
	struct sigaction sa;

	
	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		exit(1);
	}

	sa.sa_handler = SIG_IGN;
	if (sigaction(SIGPIPE, &sa, NULL) == -1) {
		exit(1);
	}

	sa.sa_handler = termination_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	
	/* Trap SIGTERM */
	if (sigaction(SIGTERM, &sa, NULL) == -1) {
		exit(1);
	}

	/* Trap SIGQUIT */
	if (sigaction(SIGQUIT, &sa, NULL) == -1) {
		exit(1);
	}

	/* Trap SIGINT */
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		exit(1);
	}
}


char *
get_iface_mac(const char *ifname)
{
	int r, s;
	struct ifreq ifr;
	char *hwaddr, mac[13];

	strcpy(ifr.ifr_name, ifname);

	s = socket(PF_INET, SOCK_DGRAM, 0);
	if (-1 == s) {
		return NULL;
	}

	r = ioctl(s, SIOCGIFHWADDR, &ifr);
	if (r == -1) {
		close(s);
		return NULL;
	}

	hwaddr = ifr.ifr_hwaddr.sa_data;
	close(s);
	snprintf(mac, sizeof(mac), "%02X%02X%02X%02X%02X%02X", 
			hwaddr[0] & 0xFF,
			hwaddr[1] & 0xFF,
			hwaddr[2] & 0xFF,
			hwaddr[3] & 0xFF,
			hwaddr[4] & 0xFF,
			hwaddr[5] & 0xFF
		);

	return strdup(mac);
}






static void
main_loop(char *gw_id)
{
	
	
	int result;
	result = pthread_create(&tid_fetch, NULL, (void *)thread_fetch, (void *)gw_id);
	if (result != 0) {
		termination_handler(0);
	}
	pthread_detach(tid_fetch);

	pause();
}

int main(int argc, char **argv) {

	init_signals();
	char *gw_id = NULL;
	while(!gw_id){
		gw_id = get_iface_mac("eth0");
		if(gw_id){
		    break;
		}		
		sleep(1);
	}
	main_loop(gw_id);
	free(gw_id);
	return(0); /* never reached */
}
