#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#define handle_error(msg) \
	    do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char *argv[]) {
	struct sockaddr_in server_addr;
	int port_no;
	int client_fd;

	if (argc < 3) {
		printf("No port inserted. \ncommand: ./client <port number>.\n");
		exit(EXIT_FAILURE);
	} else {
		port_no = atoi(argv[2]);
	}

	memset(&server_addr, 0, sizeof(struct sockaddr_in));

	/*
	 * create socket
	 */
	client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == client_fd) {
		handle_error("socket()");
	}

	/*
	 * Initialize addr for client
	 */

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port_no);
	if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
		handle_error("inet_pton");
	}

	if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
		handle_error("connect()\n");
	}
	char send_buffer[256] = {0};
	char received_buffer[256] = {0};
	system("clear");
	
	while (1) {
		int num_read, num_write;
		memset(send_buffer, 0, 256);
		memset(received_buffer, 0, 256);

		fgets(send_buffer, 256, stdin);

		num_write = send(client_fd, send_buffer, 256, 0);
		if (num_write == -1) {
			handle_error("write()");
		}
		if (strncmp("exit", send_buffer, 4) == 0) {
            break;
                }
		num_read = read(client_fd, received_buffer, 256);
		if (num_read == -1) {
            handle_error("read()");
                }
		if (strncmp("exit", received_buffer, 4) == 0) {
                break;
                }
		printf("Message from server : %s\n", received_buffer);
		
			
		sleep(1);
		}
	close(client_fd);
	return 0;
}







