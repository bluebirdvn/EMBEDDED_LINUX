#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
int main(int argc, char *argv[]) {
	int fd, count;
	if ('r' == *argv[3]) {
		fd = open(argv[1], O_RDONLY | O_CREAT);
		if (-1 == fd) {
			printf("Open failed");
			return 1;
		}
		char buffer[256] = {0};
		count = read(fd, buffer, atoi(argv[2]));
		if (-1 == count) {
			printf("Reading failed");
			close(fd);
			return 1;
		}
		printf("READ bytes: %d\n", count);
		printf("READ: %s\n", buffer);
		close(fd);
	}
	if ('w' == *argv[3]) {
		fd = open(argv[1], O_WRONLY | O_CREAT | O_APPEND);
		if (-1 == fd) {
			printf("Open (W) failed");
			return 1;
		}
		count = write(fd, argv[4], atoi(argv[2]));
		if (-1 == count) {
			printf("Writing failed");
			close(fd);
			return 1;
		}
	       if (count > 1) {
			printf("Write %d bytes to file", count);
			close(fd);
		}
	}
	return 0;
}
