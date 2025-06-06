#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

pthread_t odd, even;
int odd_count, even_count;


void *odd_count_handler(void *args) {
	odd_count = 0;
	int *arr = (int*)args;
	for (int i = 0; i < 100 ; ++i) {
		if (arr[i]%2 == 0) odd_count++;
	}
	pthread_exit(NULL);
}

void *even_count_handler(void *args) {
	even_count = 0;
	int *arr = (int*)args;
	for (int i = 0; i < 100 ; ++i) {
		if (arr[i]%2) even_count++;
	}
	pthread_exit(NULL);
}
int main() {
	int arr[100] = {0};
	for (int i = 0; i < 100; ++i) {
		arr[i] = rand()%1000;
	}
	pthread_create(&odd, NULL, &odd_count_handler, arr);

	pthread_create(&even, NULL, &even_count_handler, arr);

	pthread_join(odd, NULL);
	pthread_join(even, NULL);

	printf("mang co %d so chan.\n", odd_count);
	printf("mang co %d so le.\n", even_count);
	return 0;
}
