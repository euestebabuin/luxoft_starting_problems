#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mqueue.h>
#include <fcntl.h>
#include <sys/types.h>

#include "queues_api.h"

//MQ = message queue

// The client code is simpler and sequential. It mainly follows the code-comments. One thing to note is that 
// once the client MQ was created and an error was detected, before the program is aborted by the exit(-1)
// instruction the program tries to dispose the local MQ to avoid the OS to hodl residual MQs. This is done
// by calls to close_local_queue

char buf[BUF_SIZE];
mqd_t server_q, my_q;

// Closing and unlinking the local MQ
void close_local_queue(char *name){

	int 	rc;

	rc = mq_close(my_q);
	if (rc == -1){
		fprintf(stderr, "Couldn't close local message queue\n");
		exit(-1);
	}
	
	rc = mq_unlink(name);
	if (rc == -1){
		fprintf(stderr, "Couldn't unlink local message queue\n");
		exit(-1);
	}
}

// The name of the MQ is given as argument to the program
int main(int argc, char *argv[]){
	int 			rc;
	unsigned	 	int prio;
	
	// performing checks
	if (argc != 2){
		fprintf(stderr, "Please give only one argument: client's message queue name!\n");
		exit(-1);
	}
	
	if (argv[1][0] != '/'){
		fprintf(stderr, "A message queue must start with a forward slash '/' !\n");
		exit(-1);
	}
	// creating the MQ
	my_q = mq_open(argv[1], O_CREAT | O_RDONLY | O_EXCL, 0777, NULL);
	if (my_q == (mqd_t)-1){
		fprintf(stderr, "Couldn't create local message queue. The name of the queue must be unique!\n");
		exit(-1);
	}
	
	// opening server MQ
	server_q = mq_open(SERVER_Q_NAME, O_WRONLY, 0777, NULL);	
	if (server_q == (mqd_t)-1){
		fprintf(stderr, "Couldn't open server message queue\n");
		close_local_queue(argv[1]);
		exit(-1);
	}
	// sending the local queue name
	rc = mq_send(server_q, argv[1], strlen(argv[1]), BASIC_PRIO);
	if (rc == -1){
		fprintf(stderr, "Couldn't send message to server queue\n");
		close_local_queue(argv[1]);
		exit(-1);
	}
	// closing server MQ
	rc = mq_close(server_q);
	if (rc == -1){
		fprintf(stderr, "Couldn't close server message queue\n");
	}
	
	//receiving the server response on local MQ
	rc = mq_receive(my_q, buf, BUF_SIZE, &prio);
	if (rc == -1){
		fprintf(stderr, "Couldn't receive message from server!\n");
		close_local_queue(argv[1]);
		exit(-1);
	}
	printf("Received token number %s\n", buf);
	
	//closing the local MQ
	close_local_queue(argv[1]);
	
	return 0;
}
