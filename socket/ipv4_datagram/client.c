#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#define BUFFER_SIZE 256
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char *argv[]) {
    int server_fd, portnum;
    struct sockaddr_in client_addr;

    if (argc < 3) {
        handle_error("please input <IP address> <Port num>.");
    }

    server_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (server_fd == -1) {
        handle_error("socket()");
    }
    portnum = atoi(argv[2]);

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(portnum);
    if (inet_pton(AF_INET, argv[1], &client_addr.sin_addr) == -1) {
        handle_error("inet_pton()");
    }

    char buffer_receive[BUFFER_SIZE] = {0}, buffer_send[BUFFER_SIZE] = {0};
    int num_read, num_write;
    socklen_t len = sizeof(client_addr);
    while (1) {
        printf("Send:\n");
        fgets(buffer_send, BUFFER_SIZE, stdin);
        num_write = sendto(server_fd, buffer_send, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, len);

        if (num_write < 0) {
            handle_error("write error.");
        }

        num_read = recvfrom(server_fd, buffer_receive, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &len);

        if (num_read < 0) {
            handle_error("read error.");
        }
        if (num_read > 0) {
            printf("Mesage from server: %s\n", buffer_receive);
        }

    }
    close(server_fd);
    return 0;
    }


