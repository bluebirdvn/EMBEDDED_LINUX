#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#define LISTEN_BACKLOG 50
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char *argv[]) {
	struct sockaddr_in server_addr, client_addr;
	int port_no, opt, len;
	int server_fd, client_fd_1;

	if (argc < 2) {
		printf("No port inserted.\ncomman: ./server <port number>.\n");
		exit(EXIT_FAILURE);
	} else {
		port_no = atoi(argv[1]);
	}

	memset(&server_addr, 0, sizeof(struct sockaddr_in));
	memset(&client_addr, 0, sizeof(struct sockaddr_in));

	/*
	 * Create socket
	 * 
	 */
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == server_fd) {
		handle_error("socket()");
	}
	/*
	 * opt = 1: turn on option
	 */
	opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    	handle_error("setsockopt(SO_REUSEADDR)");
	}
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
    	handle_error("setsockopt(SO_REUSEPORT)");
	}
	/*
	 * Initialize addr for server
	 */
	/*
	 * use IPv4
	 */
	server_addr.sin_family = AF_INET;
	/*
	 * convert port_no from host byte order to networkbyte orfer
	 */
	server_addr.sin_port = htons(port_no);
	/*
	 * listen to all ip 
	 */
	server_addr.sin_addr.s_addr = INADDR_ANY;
	
	/*
	 * bind socket to server addr
	 */
	if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        	handle_error("bind()");
	}
	if (listen(server_fd, LISTEN_BACKLOG) == -1) {
		handle_error("listen()");
	}
	
	len = sizeof(client_addr);
	
	while (1) {
		printf("Server is waiting at port: %d.\n", port_no);

		client_fd_1 = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t *)&len);
		if (client_fd_1 == -1) {
			handle_error("accept()");
		}
		system("clear");
		printf("Server: got connection.\n");

		char send_buffer[256] = {0};
		char received_buffer[256] = {0};
		while(1) {	
			int num_read = read(client_fd_1, received_buffer, 256);
			if (num_read == -1) {
				handle_error("read()");
			}

			if (strncmp("exit", received_buffer, 4) == 0) {
				break;
				}

			printf("\nMessage receive: %s\n", received_buffer);

			printf("Please respond to client: \n");
			fgets(send_buffer, 256, stdin);

			int num_write = send(client_fd_1, send_buffer, 256, 0);
			if (num_write == -1) {
				handle_error("write()");
			}

			if (strncmp("exit", send_buffer, 4) == 0) {
				break;
			}
			sleep(1);
		}
		close(client_fd_1);
	}
	close(server_fd);
	return 0;
}



