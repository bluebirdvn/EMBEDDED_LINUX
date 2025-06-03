#include <stdio.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>

int main(void) {
	int fd, num_write, count = 0;
	char buffer[] = "Hello world";
	fd = open("hello.txt", O_CREAT |  O_RDWR | O_APPEND);
	if (-1 == fd) {
		printf("Open failed");
		return 1;
	}

	num_write = write(fd, buffer, sizeof(buffer));
	if (-1 == num_write) {
		printf("First time write failed");
		close(fd);
		return 1;
	}
	count += num_write;
	lseek(fd, 0, SEEK_SET);
	
	char buffer_2[] = "This is seconds time writing to this file";
	
	num_write = write(fd, buffer_2, sizeof(buffer_2));
	
	if (-1 == num_write) {
		printf("Write seconds time failed");
		close(fd);
		return 1;
	}
	count += num_write;

	if (count == 0) {
		printf("num of bytes writed: %d\n", count);
		close(fd);
		return 1;
	}
	char buffer_rd[256] = {0};
	lseek(fd, 0, SEEK_SET);
	num_write = read(fd, buffer_rd, count);
	if (-1 == num_write) {
		printf("Read failed");
		close(fd);
		return 1;
	}
	printf("READ: %s\n:", buffer_rd);
	close(fd);
	return 0;
}




	
