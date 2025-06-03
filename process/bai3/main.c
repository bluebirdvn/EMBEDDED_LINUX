#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
void signal_handler(int sig){
	printf("Child: Receive SIGUSR1, sig number %d\n", sig);
}

int main() {
	pid_t pid_child = fork();

	if (-1 == pid_child) {
		printf("Error!!\n");
		return 1;
	} else if (pid_child == 0) {
		printf("This is child process.\n");
		signal(SIGUSR1, signal_handler);
		pause();

		exit(0);

	} else {
		printf("This is parent process.\n");
		sleep(2);
		kill(pid_child, SIGUSR1);
		wait(NULL);
		printf("signal to child success.\n");
	}
	return 0;
}
