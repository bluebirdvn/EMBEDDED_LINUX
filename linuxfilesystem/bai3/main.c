#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
int main(void) {
	int fd, count;
	char buffer[] = "This is my computer and camera";
	fd = open("text.txt", O_RDWR | O_APPEND | O_CREAT);
	if (-1 == fd) {
		printf("Open failed.");
		return 1;
	}

	struct stat st;
	stat("text.txt", &st);
	
	count = write(fd, buffer, sizeof(buffer));
	if (-1 == count) {
		printf("write failed.\n");
		close(fd);
		return 1;
	}

	printf("type file: ");
	printf("file name: ");
	printf("last time: ");
	printf("file size: ");
	close(fd);
	return 0;
}


