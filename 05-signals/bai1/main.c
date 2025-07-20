#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
int count = 0;
void signal_int_handler() {
	printf("SIGINT received\n");
	count++;
	if (count == 3) {
		exit(EXIT_SUCCESS);
	}
}

int main() {
	if (signal(SIGINT, signal_int_handler) == SIG_ERR) {
		printf("can't handler SIGINT\n");
		exit(EXIT_FAILURE);
	}
	while(1);
	return 0;
}

