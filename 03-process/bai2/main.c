#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int a = 0;

void increase_a() {
	printf("Increase: %d\n", ++a);
}


int main() {
	int ret_val = fork();

	if (ret_val < 0) {
		printf("Error");
		return 1;
	} else if (ret_val == 0) {
		char *env = getenv("MYCMD");
		if (env == NULL) {
			printf("Child: MYCMD env_var not set.\n");
			exit(1);
		}

		int env_var = atoi(env);
		if (env_var == 1) {
			printf("Running ls cmd:ls -l\n");
			execlp("ls", "ls" "-l", NULL);
		} else if (env_var == 2) {
			printf("Runnig date cmd: date\n");
			execlp("date", "date", NULL);
		} else {
			printf("env_var not valid\n");
			exit(1);
		}
		printf("runnig from process 1");
		exit(1);
	} else {
		printf("runing from process parent\n");
		increase_a();
		wait(NULL);
		printf("process child finshed\n");
	}
	return 0;
}


