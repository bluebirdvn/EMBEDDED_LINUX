#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_t thread_1, thread_2;


void *thread_handler_1(void *arg) {
	printf("This is thread 1\n");
	printf("My thread ID is: %lu\n", pthread_self());
	pthread_exit(NULL);
}
void *thread_handler_2(void *arg) {
	printf("This is thread 2\n");
	printf("My thread ID is: %lu\n", pthread_self());
	pthread_exit(NULL);
}
int main() {
	pthread_create(&thread_1, NULL, &thread_handler_1, NULL);
	pthread_create(&thread_2, NULL, &thread_handler_2, NULL);
	pthread_join(thread_1, NULL);
	pthread_join(thread_2, NULL);
	return 0;
}

	
