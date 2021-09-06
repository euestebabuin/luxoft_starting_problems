#include <stdio.h>
#include <pthread.h>
#include "sl_api.h"

pthread_barrier_t barrier;
sl_list *L = NULL;

void *worker_thread_1(void *arg){
	int id = *((int*)arg);
	pthread_barrier_wait(&barrier);

	printf("Adding %d from thread %d\n", 2, id);
	add_node(&L, 2);
	printf("Adding %d from thread %d\n", 4, id);
	add_node(&L, 4);
	printf("Adding %d from thread %d\n", 10, id);
	add_node(&L, 10);
	printf("Deleting %d from thread %d\n", 2, id);
	delete_node(&L, 2);
	printf("Sorting the list\n");
	sort_list(L);
	printf("Deleting %d from thread %d\n", 10, id);
	delete_node(&L, 10);
	printf("Deleting %d from thread %d\n", 5, id);
	delete_node(&L, 5);
	return NULL;
}

void *worker_thread_2(void *arg){
	int id = *((int*)arg);
	pthread_barrier_wait(&barrier);

	printf("Adding %d from thread %d\n", 11, id);
	add_node(&L, 11);
	printf("Adding %d from thread %d\n", 1, id);
	add_node(&L, 1);
	printf("Deleting %d from thread %d\n", 11, id);
	delete_node(&L, 11);
	printf("Adding %d from thread %d\n", 8, id);
	add_node(&L, 8);
	printf("Printing the list from thread%d\n", id);
	print_list(L);
	return NULL;
}

void *worker_thread_3(void *arg){
	int id = *((int*)arg);
	pthread_barrier_wait(&barrier);

	printf("Adding %d from thread %d\n", 30, id);
	add_node(&L, 30);
	printf("Adding %d from thread %d\n", 25, id);
	add_node(&L, 25);
	printf("Adding %d from thread %d\n", 100, id);
	add_node(&L, 100);
	printf("Sorting the list\n");
	sort_list(L);
	printf("Printing the list from thread%d\n", id);
	print_list(L);
	printf("Deleting %d from thread %d\n", 100, id);
	delete_node(&L, 100);

	return NULL;
}

int main(int argc, char *argv[]){
	pthread_t t1, t2, t3;
	int tid1 = 1, tid2 = 2, tid3 = 3;

	start_list_work();
	pthread_barrier_init(&barrier, NULL, 1);
	
	pthread_create(&t1, NULL, &worker_thread_1, (void*)(&tid1));
	pthread_create(&t2, NULL, &worker_thread_2, (void*)(&tid2));
	pthread_create(&t3, NULL, &worker_thread_3, (void*)(&tid3));

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);

	print_list(L);
	flush_list(&L);

	stop_list_work();

	return 0;
}
