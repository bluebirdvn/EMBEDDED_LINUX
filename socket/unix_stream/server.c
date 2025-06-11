#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/un.h>
#include <string.h>

#define handle_error(msg) do {perror(msg);exit(EXIT_FAILURE);} while (0)
#define LISTEN_BACKLOG 10
#define SOCKPATH "/tmp/mysock"
#define BUFFERSIZE 256

int main() {
    struct sockaddr_un server_addr;
    int server_fd, client_fd;
    
    unlink(SOCKPATH);

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (server_fd == -1) {
        handle_error("socket init");
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKPATH, sizeof(server_addr.sun_path)-1);
    socklen_t len = sizeof(server_addr);
    if (bind(server_fd, (struct sockaddr*)&server_addr, len) == -1) {
        handle_error("bind()");
    }

    if (listen(server_fd, LISTEN_BACKLOG) == -1) {
        handle_error("listen()");
    }

    char send_buffer[BUFFERSIZE] = {0}, receive_buffer[BUFFERSIZE] = {0};
    int sen_num, rec_num;
    client_fd = accept(server_fd, (struct sockaddr*)&server_addr, &len);
    if (client_fd == -1) {
        handle_error("accept()");
    }
    system("clear");
    while (1)
    {
        
        printf("Getting message from clien: \n");
        rec_num = read(client_fd, receive_buffer, BUFFERSIZE);
        if (rec_num < 0) {
            printf("No data from client.\n");
        } else if (rec_num >= 1) {
            printf("Message from client: %s\n", receive_buffer);
        }

        printf("Message to client: ");

        fgets(send_buffer, BUFFERSIZE, stdin);

        sen_num = write(client_fd, send_buffer, BUFFERSIZE);

        if (sen_num < 0) {
            handle_error("write()");
        }

    }
    close(client_fd);
    close(server_fd);
    return 0;
}
    