#include "init.h"

volatile sig_atomic_t ready = 0;

void sig_handler(int signum) {
    (void)signum;
    ready = 1;
}


int main() {
    int fds[2];
    if (pipe(fds) == -1) {
        handle_error("pipe init failed.");
    }
    pid_t thr = fork();

    char send_buffer[BUFFERSIZE] = {0};
    char recv_buffer[BUFFERSIZE] = {0};
    
    if (thr == -1) {
        handle_error("Create child process failed.");
    } else if (thr == 0) {
        // Child process
        if (close(fds[1]) == -1) {
            handle_error("Close write pipe from child failed.");
        }
        
        struct sigaction sig;
        sig.sa_handler = sig_handler;
        sigemptyset(&sig.sa_mask);
        sig.sa_flags = SA_RESTART;
        if (sigaction(SIGUSR1, &sig, NULL) == -1) {
            handle_error("sigaction.");
        }
        
        printf("Child waiting for signal...\n");
        while(!ready) {
            pause();
        }

        printf("Child received wake-up signal\n");
        printf("This is child process.\n");

        while (1) {
            int num = read(fds[0], recv_buffer, sizeof(recv_buffer) - 1);
            
            if (num == -1) {
                handle_error("read failed");
            } else if (num == 0) {
                printf("Pipe closed by parent\n");
                break;
            } else {
                recv_buffer[num] = '\0';
                
                if (strncmp("exit", recv_buffer, 4) == 0) {
                    printf("Child received exit command. Exiting...\n");
                    break;
                }
                printf("Message from parent: %s", recv_buffer);
                fflush(stdout);
            }
        }
        close(fds[0]);
        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        printf("This is parent process. Child PID: %d\n", thr);
        sleep(1);
        
        if (close(fds[0]) == -1) {
            handle_error("close read pipe from parent failed");
        }
        if (kill(thr, SIGUSR1) == -1) {
            handle_error("kill");
        }
    
        while (1) {    
            fflush(stdout);
            if (!fgets(send_buffer, sizeof(send_buffer), stdin)) {
                break;
            }
            
            int len = strlen(send_buffer);
            
            if (write(fds[1], send_buffer, len) != len) {
                handle_error("write failed");
            }
            
            if (strncmp("exit", send_buffer, 4) == 0) {
                break;
            }
        }
        close(fds[1]);
        wait(NULL);
        exit(EXIT_SUCCESS);
    }
}