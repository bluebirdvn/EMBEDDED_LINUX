#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(int args, char *argv[]) {
	pid_t pid_child = fork();
	if (-1 == pid_child) {
		printf("Error!!!\n");
		return 1;
	} else if (0 == pid_child) {
		printf("This is child process pid: %d.\n", getpid());
		for (int i = 0; i < args; ++i) {
			sleep(1);
			printf("argv[%d] : %s\n", i, argv[i]);
		}
		sleep(1);
		//while(1);
		exit(43);
	} else {
		int rv, status;
		printf("This is parent process\n");
		rv = wait(&status);
		if (-1 == rv) {
			printf("wait() unsuccessfull\n");
			return 1;
		}

		printf("This is PID of child process %d\n", rv);
		if (WIFEXITED(status)) {
			printf("normally exit status: %d\n", WEXITSTATUS(status));
		} else if (WIFSIGNALED(status)) {
			printf("killed by signal, value: %d\n", WTERMSIG(status));
		}
	}
	return 0;
}

