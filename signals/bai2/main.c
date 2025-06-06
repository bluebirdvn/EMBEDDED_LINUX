#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
int second = 0;

void alarm_handler() {
	printf("Timer: %d seconds\n", ++second);
	if (second == 10) {
		exit(EXIT_SUCCESS);
	
	}
	alarm(1);
}

int main() {
	if (signal(SIGALRM, alarm_handler) == SIG_ERR) {
		printf("Error init signal\n");
		exit(EXIT_FAILURE);
	}
	alarm(1);
	while(1) {
		pause();
	}
	
	return 0;
}

