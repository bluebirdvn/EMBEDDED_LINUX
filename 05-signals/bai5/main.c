#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

void sigint_handler() {
	printf("SIGINT received.\n");
}

void sigterm_handler() {
	printf("SIGTERM received.\n");
	exit(EXIT_SUCCESS);
}


int main() {
	signal(SIGINT, sigint_handler);
	signal(SIGTERM, sigterm_handler);
	fd_set readfds;
	char buffer[100];
	while(1) {
	
	struct timeval timeout;	
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	FD_ZERO(&readfds);
	FD_SET(STDIN_FILENO, &readfds);
	
	printf("waiting for input 5s....\n");
	int result = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);
	if (-1 == result) {
		if (errno == EINTR) {
			continue;
		}else {
			printf("select error");
			exit(EXIT_FAILURE);
		}
	} else if (result == 0) {
		printf("timeout....\n");
	} else if (FD_ISSET(STDIN_FILENO, &readfds)) {
		if (fgets(buffer, sizeof(buffer), stdin)){
			printf("your input: %s", buffer);
		}
	}
	}
	return 0;
}
