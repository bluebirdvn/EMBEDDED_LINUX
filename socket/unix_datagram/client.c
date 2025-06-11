#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/un.h>
#include <string.h>

#define handle_error(msg) do {perror(msg);exit(EXIT_FAILURE);} while (0)
#define SOCKPATH "/tmp/mysock"
#define CLINETPATH "/tmp/clientsock"
#define BUFFERSIZE 256

int main() {
    struct sockaddr_un server_addr, client_addr;
    int client_fd;
    socklen_t len;
    client_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (client_fd == -1) {
        handle_error("socket()");
    }
    unlink(CLINETPATH);
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sun_family = AF_UNIX;
    strncpy(client_addr.sun_path, CLINETPATH, sizeof(client_addr.sun_path) - 1);
    len = sizeof(client_addr);
    if (bind(client_fd, (struct sockaddr*)&client_addr, len) == -1) {
        handle_error("bind()");
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKPATH, sizeof(server_addr.sun_path) - 1);
    char send_buffer[BUFFERSIZE] = {0}, recv_buffer[BUFFERSIZE] = {0};

    int send_num, read_num;
    len = sizeof(server_addr);
    while (1)
    {
        printf("Send message to server: ");
        fgets(send_buffer, BUFFERSIZE, stdin);
        send_num = sendto(client_fd, send_buffer, BUFFERSIZE, 0, (struct sockaddr*)&server_addr, len);
        if (send_num == -1) {
            handle_error("sendto()");
        }

        read_num = recvfrom(client_fd, recv_buffer, BUFFERSIZE, 0, (struct sockaddr*)&server_addr, &len);
        if (read_num == -1) {
            handle_error("recvfrom()");
        }
        printf("Receive message from server: %s\n", recv_buffer);
    }
    close(client_fd);
    return 0;


}