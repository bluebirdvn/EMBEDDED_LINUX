#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>

#define BUFFER_SIZE 256
#define handle_error(msg) do {perror(msg), exit(EXIT_FAILURE);} while (0)
int ready = 0;
void sig_handler(int signo) {
    (void)signo;
    ready = 1;
}
int main() {
    int pd[2];
    if (pipe(pd) == -1) {
        handle_error("pipe error");
    }
    pid_t pid;
    pid = fork();

    char send_buffer[BUFFER_SIZE] = {0}, read_buffer[BUFFER_SIZE] = {0};

    if (pid == -1) {
        handle_error("pid error");
    } else if (pid == 0) {
        struct sigaction sig;
        sig.sa_handler = sig_handler;
        sigemptyset(&sig.sa_mask);
        sig.sa_flags = SA_RESTART;

        if (sigaction(SIGUSR1, &sig, NULL) == -1) {
            handle_error("sigaction.");
        }

        printf("Child waiting for signal...\n");
        while (!ready) {
            pause();
        }
        printf("This is child process. My pid %d, my father %d", getpid(), getppid());
        if (close(pd[1]) == -1) {
            handle_error("close write from child failed");
        }
        int num_read = read(pd[0], read_buffer, BUFFER_SIZE);

        if (num_read == -1) {
            handle_error("read");
        }
        int size = strlen(read_buffer);

        printf("So ky tu doc tu pipe la: %d\n", size-1);
        printf("Msg from parent: %s\n", read_buffer);
        close(pd[0]);

        exit(EXIT_SUCCESS);

    } else {
        printf("This is parent process. My pid: %d\n", getpid());

        close(pd[0]);
        printf("Msg to child: ");
        fgets(send_buffer, BUFFER_SIZE, stdin);

        int num_write = write(pd[1], send_buffer, BUFFER_SIZE);

        if (num_write == -1) {
            handle_error("write");
        }
        kill(pid, SIGUSR1);
        close(pd[1]);
        wait(NULL);
        exit(EXIT_SUCCESS);
    }
 }