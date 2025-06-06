#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
void sig_handler(int sig) { printf("Ignore SIGTSTP");
}


int main() {
	signal(SIGTSTP, sig_handler);
	while(1){
		sleep(1);
	}
	return 0;
}

