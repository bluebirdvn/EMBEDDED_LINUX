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
    struct sockaddr_un server_addr, client_addr;
    int server_fd;
    socklen_t len;
    server_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    unlink(SOCKPATH);
    if (server_fd == -1) {
        handle_error("socket()");
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKPATH, sizeof(server_addr.sun_path) - 1);
    len = sizeof(server_addr);
    if (bind(server_fd, (struct sockaddr*)&server_addr, len) == -1) {
        handle_error("bind()");
    } 
    char send_buffer[BUFFERSIZE] = {0}, read_buffer[BUFFERSIZE] = {0};
    len = sizeof(client_addr);
    printf("len is: %d\n", len);
    while (1) {
        len = sizeof(client_addr);
        memset(send_buffer, 0, BUFFERSIZE);
        memset(read_buffer, 0, BUFFERSIZE);
        int read_num = recvfrom(server_fd, read_buffer, BUFFERSIZE, 0, (struct sockaddr*)&client_addr, &len);
        if (read_num == -1) {
            handle_error("read()");
        }
        printf("len is: %d\n", len);
        printf("Receive from client: %s\n", read_buffer);
        printf("Send message to client: ");
        fgets(send_buffer, BUFFERSIZE, stdin);
        int send_num = sendto(server_fd, send_buffer, BUFFERSIZE, 0, (struct sockaddr*)&client_addr, len);
        if (send_num == -1) {
            handle_error("sendto()");
        }
    }
    close(server_fd);
    return 0;
}