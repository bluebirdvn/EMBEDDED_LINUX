#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

#define handle_error(msg) do {perror(msg); exit(EXIT_FAILURE);} while (0)

#define BUFFER_SIZE 255
#define PROCESS_NUM 2

int ready1 = 0;
int ready2 = 0;

void sig_handler(int signo) {
    (void)signo;
    ready1 = 1;
}

void sig_handler2(int signo) {
    (void)signo;
    ready2 = 1;
}

int main() {
    int pd_1[2], pd_2[2];
    if (pipe(pd_1) == -1 || pipe(pd_2) == -1) {
        handle_error("pipe()");
    }

    pid_t child1 = fork();
    if (child1 == -1) {
        handle_error("Create process 1 failed.\n");
    }
    if (child1 == 0) {
        // Child 1
        struct sigaction sig;
        sig.sa_handler = sig_handler;
        sigemptyset(&sig.sa_mask);
        sig.sa_flags = SA_RESTART;

        if (sigaction(SIGUSR1, &sig, NULL) == -1) {
            handle_error("sigaction.");
        }

        printf("Child 1 waiting for signal...\n");
        while (!ready1) {
            pause();
        }

        printf("I'm child process, my pid: %d, my father: %d\n", getpid(), getppid());

        close(pd_1[1]);
        close(pd_2[0]);

        char read_buffer[BUFFER_SIZE] = {0};
        int read_num = read(pd_1[0], read_buffer, BUFFER_SIZE);
        if (read_num == -1) {
            handle_error("read from parent failed.");
        }

        printf("Message from father: %s\n", read_buffer);
        printf("add message to child 2: \n");

        char buffer[100] = {0};
        fgets(buffer, 100, stdin);
        buffer[strcspn(buffer, "\n")] = 0;
        strcat(read_buffer, " ");           
        strcat(read_buffer, buffer);

        if (write(pd_2[1], read_buffer, BUFFER_SIZE) == -1) {
            handle_error("write to child 2 failed");
        }

        // Send signal to child 2

        close(pd_1[0]);
        close(pd_2[1]);
        exit(EXIT_SUCCESS);
    }
    pid_t child2 = fork();
    if (child2 == -1) {
        handle_error("Create process 2 failed.\n");
    }
    if (child2 == 0) {
        // Child 2
        struct sigaction sig1;
        sig1.sa_handler = sig_handler2;
        sigemptyset(&sig1.sa_mask);
        sig1.sa_flags = SA_RESTART;

        if (sigaction(SIGUSR2, &sig1, NULL) == -1) {
            handle_error("sigaction.");
        }

        printf("Child 2 waiting for signal...\n");
        while (!ready2) {
            pause();
        }

        printf("I'm 2nd child process, my pid: %d, my father: %d\n", getpid(), getppid());

        close(pd_2[1]);
        close(pd_1[0]);
        close(pd_1[1]);

        char read_buffer1[BUFFER_SIZE] = {0};
        int read_num1 = read(pd_2[0], read_buffer1, BUFFER_SIZE);
        if (read_num1 == -1) {
            handle_error("read from child 1 failed.");
        }

        printf("Message from child 1: %s\n", read_buffer1);
        close(pd_2[0]);
        exit(EXIT_SUCCESS);
    }

    // Father process
    printf("I'm father process, my pid: %d\n", getpid());
    printf("message to child 1: \n");

    close(pd_1[0]);
    close(pd_2[0]);
    close(pd_2[1]);

    char send_buffer[BUFFER_SIZE] = {0};
    fgets(send_buffer, BUFFER_SIZE, stdin);

    if (write(pd_1[1], send_buffer, BUFFER_SIZE) == -1) {
        handle_error("send to child 1 failed.");
    }

    kill(child1, SIGUSR1);
    close(pd_1[1]);

    waitpid(child1, NULL, 0);
    kill(child2, SIGUSR2);
    waitpid(child2, NULL, 0);

    return 0;
}
