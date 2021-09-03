#include <stdio.h>
#include <pthread.h>

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
	pthread_t p1, p2;

	printf("start\n");
	
	pthread_create(&p1, NULL, mythread, "Alalala");
	pthread_create(&p2, NULL, mythread, "blblblblb");

	printf("hihihihi\n");
	pthread_join(p1, NULL);
	pthread_join(p2, NULL);

	printf("end\n");

	receiver(&printer, "salut salut da da da");

	return 0;
}
