#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	int a = 0;
	++a;
	pid_t pid = fork();
	if (pid < 0) {
		printf("Fork failed.\n");
		return 1;
	} else if (pid == 0) {
		printf("This is the child process ID: %d\n", getpid());
	} else {
		printf("This is parent process, child PID: %d\n", pid);
	}
	return 0;
}

