#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define MAXPARTS 4
#define MAXNUM 1000000

pthread_t thr[] = {1,2,3,4};
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
unsigned long sum = 0;
int array[MAXNUM] = {0};

typedef struct {
	int start, end;
} args_t;


void *sum_handler(void *args) {
	int sum_t = 0;
	args_t *t = (args_t*)args;
	for (int i = t->start; i < t->end; ++i) {
		sum_t+= array[i];
	}
	pthread_mutex_lock(&lock);
	sum += sum_t;
	printf("thread %lu\n", pthread_self());
	free(t);
	pthread_mutex_unlock(&lock);
	pthread_exit(NULL);
}



int main() {
	srand(time(NULL));
	for (int i = 0; i < MAXNUM; ++i) {
		array[i] = rand()%10;
	}
	int partsize = MAXNUM/MAXPARTS;
	for (int i = 0; i < MAXPARTS; ++i) {
		args_t* t = (args_t*)malloc(sizeof(args_t));
		t->start = i*partsize;
		t->end = (i == MAXPARTS-1)?MAXNUM:(partsize+t->start);
		pthread_create(&thr[i], NULL, &sum_handler, t);
	}
	for (int i = 0; i < MAXPARTS; ++i) {
		pthread_join(thr[i], NULL);
	}

	printf("sum of array: %lu\n", sum);

	return 0;
}
