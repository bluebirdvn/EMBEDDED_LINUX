#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int counter = 0;
void *thread_handler_1(void *arg) {
	for (int i = 0; i < 1000000; ++i) {
		pthread_mutex_lock(&lock);
		counter++;
		printf("counter is: %d\n", counter);
		pthread_mutex_unlock(&lock);
	}
	pthread_exit(0);
	}

int main() {
	pthread_t thr[3] = {1, 2, 3};
	
	int ret;

	for (int i = 0; i < 3; ++i) {
		
		ret = pthread_create(&thr[i], NULL, &thread_handler_1, NULL);
		if (ret) {
			printf("Thread create failed!.\n");
			return 1;
		}
	}
	for (int j = 0; j < 3; ++j) {
		pthread_join(thr[j], NULL);
	}
	return 0;
}











