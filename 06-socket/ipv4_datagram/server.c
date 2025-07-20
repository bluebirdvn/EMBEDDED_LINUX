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

    struct sockaddr_in server_addr, client_addr;
    int port_no, opt;
    int server_fd;

    if (argc < 2) {
        handle_error("PORT number is not inserted.<import port numeber>\n");
    }

    port_no = atoi(argv[1]);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_no);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    server_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (server_fd == -1) {
        handle_error("socket()_server_fd.\n");
    }

	opt = 1;

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    	handle_error("setsockopt(SO_REUSEADDR)");
	}
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
    	handle_error("setsockopt(SO_REUSEPORT)");
	}

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        handle_error("bind()");
    }

    char buffer_send[BUFFER_SIZE] = {0}, buffer_received[BUFFER_SIZE] = {0};
    int num_read, num_write;
    socklen_t len = sizeof(client_addr);
    system("clear");

    while (1)
    {
        
        memset(buffer_send, 0, BUFFER_SIZE);
        memset(buffer_received, 0, BUFFER_SIZE);

        printf("Receive:\n");
        num_read = recvfrom(server_fd, buffer_received, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &len);

        if (num_read < 0) {
            handle_error("read error");
        } 
        if (num_read >= 1) {
            printf("Mesage from client: %s\n", buffer_received);
        }
        
        printf("Send:\n");
        fgets(buffer_send, BUFFER_SIZE, stdin);
        num_write = sendto(server_fd, buffer_send, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, len);

        if(num_write < 0) {
            handle_error("write error");
        }

    }
    close(server_fd);
    return 0;

}