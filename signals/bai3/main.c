#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
int count = 0;
void sig_handler() {
	printf("Received signal from parent.\n");
	if (++count == 5) {
		exit(EXIT_SUCCESS);
	}
}
int main() {
	pid_t ret_val = fork();
	if (-1 == ret_val) {
		printf("process init failed\n");
		return 1;
	} else if (0 == ret_val) {
		printf("This is child process, my id: %d\n", getpid());
		signal(SIGUSR1, sig_handler);
		
		while(1){ 
			pause();
		}
	} else {
		printf("This is parent process\n");
		for (int i = 0; i < 5; ++i) {
			sleep(2);
			kill(ret_val, SIGUSR1);
		}
		wait(NULL);
	}
	return 0;
}

