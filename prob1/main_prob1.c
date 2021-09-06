#include <stdio.h>
#include <pthread.h>
#include "sl_api.h"

void printer(void *arg){
	printf("%s\n", (char*) arg);
}

void receiver(void (*ptr_func)(void *arg), void *arg){
	(*ptr_func)(arg);
}

void *mythread(void *arg){
	printf("%s\n", (char*) arg);
	return NULL;
}

int main(int argc, char *argv[]){
/*	pthread_t p1, p2;

	printf("start\n");
	
	pthread_create(&p1, NULL, mythread, "Alalala");
	pthread_create(&p2, NULL, mythread, "blblblblb");

	printf("hihihihi\n");
	pthread_join(p1, NULL);
	pthread_join(p2, NULL);

	printf("end\n");

	receiver(&printer, "salut salut da da da");
*/
	sl_list *test = NULL;
	add_node(&test, 5);
	add_node(&test, -3);
	add_node(&test, 12);
	add_node(&test, 8);
	add_node(&test, 4);
	sort_list(test);
	print_list(test);
	delete_node(&test, -3);
	print_list(test);
	delete_node(&test, 12);
	print_list(test);
	flush_list(&test);
	print_list(test);
	return 0;
}
