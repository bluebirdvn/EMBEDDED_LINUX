#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>


int main(int args, char *argv[]) {
	pid_t pid_0 = fork();
	if (-1 == pid_0) {
		printf("create child process unsuccessfully");
		return 1;
	} else if ( 0 == pid_0) {
		printf("This is child process %d\n", getpid());
		int fd, re_val;
		fd = open(argv[1], O_WRONLY | O_APPEND | O_CREAT, 0664);
		if (-1 == fd) {
			printf("open unsuccessfully\n");
			return 1; 
			}
		re_val = write(fd, argv[2], strlen(argv[2]));
        	if (-1 == re_val){
			printf("write unsuccessfully");
			close(fd);
 			return 1;
        		}
		printf("writing %d bytes in file %s\n", re_val, argv[1]);
		close(fd);
		printf("My PID(child): %d, My father: %d\n", getpid(), getppid());
		//cho cho tien trinh cha ket thuc (neu xet trong truong hop tien trinh mo coi)
		sleep(5);
		printf("My PID(child): %d, My father: %d\n", getpid(), getppid());
		sleep(10);
		exit(0);
	} else {
		//while(1) se tao tien trinh zombie
		//while(1);
		//wait(NULL);
		sleep(1);
		printf("This is parent process\n");
		int fd1, re_val1;
		fd1 = open(argv[1], O_RDONLY) ;
		if (-1 == fd1) {
			printf("Open from parent unsuccess\n");
			return 1;
		}
		char buffer[256] = {0};
		struct stat st;
		fstat(fd1, &st);
		int size_fd = st.st_size;
		re_val1 = read(fd1, buffer,size_fd+1);
		if (-1 == re_val1) {
			printf("read unsuccesfully\n");
			close(fd1);
			return 1;
		}
		printf("read %d bytes\n", re_val1);
		printf("%s\n", buffer);
		close(fd1);
	}
	return 0;
}



