#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include <mqueue.h>
#include <fcntl.h>
#include <sys/types.h>

#include "queues_api.h"

// MQ = message Queue

// The server application is split into two threads: the main thread, taking commands from the cli, thus 
// being able to be instructed when to stop, and the token handler thread. Both of these two threads run
// while loops in order to provide server like functionality. However, while the main loop closes itself
// using the main_active variable, the token handler thread presents a difficult situation on closing,
// situation which will be described below.

pthread_mutex_t 	cancel_lock;
mqd_t 			m;



// This is the token_handler function, run in a separate thread. It continuously reads the server MQ, 
// and sends back a token number to the client through its provided private MQ. The main problem is
// closing this thread once the main thread receives the 'exit' command. Since mq_receive is a blocking
// call, you usually cannot cancel a blocked thread. Also, interrupting a token number processing is
// undesirable, so a mutex was added to secure this critical zone from closing the thread. In order to 
// close a blocked thread, it had to be set to PTHREAD_CANCEL_ENABLE and then pthread_cancel was called 
// from the main thread.
//
// An alternative solution would have been setting the server que to O_NONBLOCK and leave the token handler
// thread in a busy waiting state, checking the queue and a while condition at all times.	

void *token_handler(void *arg){
	int 		rc, token_number = 0;
	unsigned int 	prio;
	char 		buf[BUF_SIZE];
	char		token_string[20];
	mqd_t		client_q;
	
	// making this thread cancelable
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &rc);
	while (1){
		// read server MQ. Here usualyy this threads resides, blocked
		rc = mq_receive(m, buf, BUF_SIZE, &prio);
		
		if (rc != -1){
			// locking the processing zone from closing the thread
			pthread_mutex_lock(&cancel_lock);
			
			//sending the client a new token number
			printf("\nNew token request arrived from %s", buf);		
			client_q = mq_open(buf, O_WRONLY, 0777, NULL);
			if (client_q == (mqd_t)-1){
				fprintf(stderr, "Couldn't open client queue\n");
				continue;
			}
			token_number++;
			sprintf(token_string, "%d", token_number);
			mq_send(client_q, token_string, strlen(token_string), BASIC_PRIO);
			mq_close(client_q);			
			printf("\nSent token number %d", token_number);	
			// unlocking the thread. Now it can be canceled		
			pthread_mutex_unlock(&cancel_lock);
		}
		
	}
	return NULL;
}

// This function initializes the MQ, the cancel mutex and starts the token handler thread

void initialize_server(pthread_t *tk_h){

	m = mq_open(SERVER_Q_NAME, O_CREAT | O_RDONLY, 0777, NULL);	
	if (m == (mqd_t)-1){
		fprintf(stderr, "Couldn't create server message queue\n");
		exit(-1);
	}	
	pthread_mutex_init(&cancel_lock, NULL);
	pthread_create(tk_h, NULL, &token_handler, NULL);
}

// This function closes the server by making sure the token handler thread was canceled and by closing and
// unlinking the MQ
void close_server(pthread_t tk_h){
	int 	rc;
	
	printf("\nWaiting for token handling completion ... \n");
	pthread_join(tk_h, NULL);
	printf("Closing the server ... \n");
	pthread_mutex_destroy(&cancel_lock);
	
	rc = mq_close(m);
	if (rc == -1){
		fprintf(stderr, "Couldn't close server message queue\n");
		exit(-1);
	}
	rc = mq_unlink(SERVER_Q_NAME);
	if (rc == -1){
		fprintf(stderr, "Couldn't unlink server message queue\n");
		exit(-1);
	}
}

int main(int argc, char *argv[]){
	pthread_t 	tk_h;
	int 		main_active = 1;
	char		input_buf[100];
	
	initialize_server(&tk_h);	
	printf("Server started! Type 'exit' to quit.\n>");
	while (main_active == 1){
		// Sorry that I used scanf. I know it's unsafe but I couldn't find anything else :(
		scanf("%s", input_buf);
		if(strncmp("exit", input_buf, 4) == 0){
			// closing the main thread
			main_active = 0;
			// closing the token handler thread by attempting to take the mutex
			pthread_mutex_lock(&cancel_lock);
			pthread_cancel(tk_h);
			pthread_mutex_unlock(&cancel_lock);
		} 
		printf(">");
	}
	close_server(tk_h);
	
	return 0;
}
