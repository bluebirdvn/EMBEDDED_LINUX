#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/un.h>
#include <string.h>

#define handle_error(msg) do {perror(msg);exit(EXIT_FAILURE);} while (0)

#define SOCKPATH "/tmp/mysock"
#define BUFFERSIZE 256

int main() {
    struct sockaddr_un server_addr;
    int client_fd;

    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) {
        handle_error("socket()");
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKPATH, sizeof(server_addr.sun_path)-1);

    socklen_t len = sizeof(server_addr);
    if(connect(client_fd, (struct sockaddr*)&server_addr, len) == -1) {
        handle_error("connect()");
    }
    char send_buffer[BUFFERSIZE] = {0}, read_buffer[BUFFERSIZE] = {0};

    while (1) {
        printf("send message to server: \n");
        fgets(send_buffer, BUFFERSIZE, stdin);
        int num_write = write(client_fd, send_buffer, BUFFERSIZE);
        if (num_write < 0) {
            handle_error("write()");
        }
        printf("message receive from server: \n");

        
        int num_read = read(client_fd, read_buffer, BUFFERSIZE);
        if (num_read < 0) {
            handle_error("read()");
        }
        if (num_read > 0) {
            printf("%s\n", read_buffer);
        }
    }
    close(client_fd);

    return 0;

}