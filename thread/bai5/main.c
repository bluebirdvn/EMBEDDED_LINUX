#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

pthread_t r1, r2, r3, r4, r5, w1, w2;
int count;
pthread_rwlock_t rw_lock = PTHREAD_RWLOCK_INITIALIZER;

void *read_data_handler(void *args) {
	pthread_rwlock_rdlock(&rw_lock);
	printf("Data: %d\n", count);
	pthread_rwlock_unlock(&rw_lock);
	pthread_exit(NULL);
}

void *write_data_handler(void *args) {
	pthread_rwlock_wrlock(&rw_lock);
	count++;
	printf("Increase Data.\n");
	pthread_rwlock_unlock(&rw_lock);
	pthread_exit(NULL);
}

int main() {
	pthread_create(&r1, NULL, &read_data_handler, NULL);
	pthread_create(&r2, NULL, &read_data_handler, NULL);
	pthread_create(&w2, NULL, &write_data_handler, NULL);
	pthread_create(&r4, NULL, &read_data_handler, NULL);
	pthread_create(&r5, NULL, &read_data_handler, NULL);
	pthread_create(&w1, NULL, &write_data_handler, NULL);
	pthread_create(&r2, NULL, &read_data_handler, NULL);
	pthread_join(r1, NULL);
	pthread_join(r2, NULL);
	pthread_join(r3, NULL);
	pthread_join(r4, NULL);
	pthread_join(r5, NULL);
	pthread_join(w1, NULL);
	pthread_join(w2, NULL);

	printf("The value of count: %d\n", count);
	return 0;
}








