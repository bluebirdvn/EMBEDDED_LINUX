#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

int data;
int data_rd;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ready = PTHREAD_COND_INITIALIZER;
pthread_t producer, consumer;


void *producer_handler(void *args) {
	for (int i = 0; i < 10; ++i) {
		pthread_mutex_lock(&lock);
		while(data_rd) {
			pthread_cond_wait(&ready, &lock);
		}
		srand(time(NULL));
		data = rand()%100 + 1;
		printf("goods: %d\n", data);
		sleep(1);
		data_rd = 1;
		pthread_cond_signal(&ready);
		pthread_mutex_unlock(&lock);
	}
	return NULL;
}

void *consumer_handler(void *args) {
	for (int i = 0; i < 10; ++i) {
		pthread_mutex_lock(&lock);
		while(!data_rd){
			pthread_cond_wait(&ready, &lock);
		}
		data_rd = 0;
		printf("Take goods!! goods remain: %d\n", data);
		data = 0;
		sleep(1);
		pthread_cond_signal(&ready);
		pthread_mutex_unlock(&lock);
	}
	return NULL;
}


int main() {
	data_rd = 0;
	pthread_create(&producer, NULL, &producer_handler, NULL);
	pthread_create(&consumer, NULL, &consumer_handler, NULL);

	pthread_join(producer, NULL);
	pthread_join(consumer, NULL);

	return 0;
}


