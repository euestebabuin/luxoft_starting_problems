#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <pthread.h>

#include "socket_api.h"

/*
* 	The server works using a mixture of the select statement and thread syncrhonization. The main loop of the server listens for
* two possible inputs, using the select() function: the keyboard (which triggers server shutdown) or for incoming connections on the 
* listening port. A new connection is handled in the accept_connection() function which in turn creates a new thread to handle file 
* transmission, the send_file() function.
*
* Thread synchronization is not necesary until the closing of the server. Initially there are 100 slots open for clients.
* Each time a client is being served, the server decrements a slot as occupied in the accept_connection() function. Once the serving 
* ends, the client increments a slot. When the servers is ending, it tries to mark all possible slots as occupied. If any client is 
* still being  served, the server operation is blocked until client handling completion. This functionality was implemented using the 
* semaphore thread_sem.
*/

// how many pending connection from clients there can be
#define QUEUE_SIZE_LIMIT 100

int 	listen_socket;
sem_t	thread_sem;

// function used to properly close the server. It closes the listening socket, it waits for client handling completion and destroys the
// semaphore
void close_server(){
	int 	status, i;
	
	printf("\nClosing the server ... \n");
	status = close(listen_socket);
	THE_END(status == -1, "Cannot close listening socket");	
	
	printf("Waiting for client handling to be done ... \n");
	for(i = 0; i < MAX_CLIENTS; i++){
		sem_wait(&thread_sem);
	}
	status = sem_destroy(&thread_sem);
	THE_END(status == -1, "Cannot destroy sempahore");	
	
	printf("Server down\n");
}

//this function is used to initiazlie the server. It opens and binds the listening socket and starts listening
void start_server(char *server_port_input){

	int 			status, server_port;
	struct sockaddr_in	listen_address;
	
	//parsing for the port number
	server_port = strtol(server_port_input, NULL, 10);
	THE_END(server_port == 0, "Invalid server port");
	
	status = sem_init(&thread_sem, 0, MAX_CLIENTS);
	THE_END(status == -1, "Cannot initialize semaphore");

	//creating the socket
	printf("Starting the server\n");
	listen_socket = socket(PF_INET, SOCK_STREAM, 0);
	THE_END(listen_socket == -1, "Cannot create listening socket");
	
	listen_address.sin_family = AF_INET;
	listen_address.sin_addr.s_addr = htonl(INADDR_ANY);
	listen_address.sin_port = htons(server_port);
	
	//binding the socket
	status = bind(listen_socket, (struct sockaddr *) &listen_address, sizeof(listen_address));
	THE_END_CLEANUP(status == -1, "Couldn't bind listening socket", close_server);
	
	//starts listening for incoming connections
	status = listen(listen_socket, QUEUE_SIZE_LIMIT);
	THE_END_CLEANUP(status == -1, "Couldn't start listening", close_server);
	printf("Listening for new connections. Press any key to close the server\n");
}

// function needed for special client connection closing. It is required since closing the thread requires multiple operations such as
// incrementing back the semaphore, closing the communicating socket and freeing it's memory
void close_client_handling(char *ret_msg, int *comm_socket){
	printf("%s", ret_msg);
	//stopped serving
	sem_post(&thread_sem);
	close(*comm_socket);
	free(comm_socket);
}

//function which runs in a separate thread, serving a client
void *send_file(void *arg){
	int 		status, count_read, *comm_socket;
	char		file_name[BUFFER_SIZE];
	char		fnf[] = "FILE NOT FOUND!";
	char		buffer[BUFFER_SIZE];
	FILE		*f;
	
	//retrieving comm socket from arguments
	comm_socket = ((int*)arg);
	
	//receiving file name
	status = recv(*comm_socket, file_name, BUFFER_SIZE, 0);
	if(status == -1){
		close_client_handling("Couldn't receive file name from client", comm_socket);
		return ((void*)-1);
	}
	
	file_name[status] = '\0';
	//checking file name
	printf("Looking for file %s ...\n", file_name);
	f = fopen(file_name, "rb");
	if (f == NULL){
		status = send(*comm_socket, fnf, strlen(fnf), 0);
		close_client_handling("File not found! Closing connection!\n", comm_socket);
		return ((void*)-1);
	}
	
	//sending "OK", as confirmation message back to the client
	printf("Found file %s. Sending confirmation ...\n", file_name);	
	status = send(*comm_socket, "OK", 2, 0);
	if(status == -1){
		close_client_handling("Couldn't send confirmation to client", comm_socket);
		return ((void*)-1);
	}
	
	//sending the file. Since not always the maximum BUFFER_SIZE is read from the file, we only send the amount read, which is
	//count_read. Also, '\0' is added at the end of the received buffer since it is not transmitted through or added by the socket
	printf("Sending file %s\n", file_name);
	while((count_read = fread(buffer, 1, BUFFER_SIZE, f)) > 0){
		buffer[count_read] = '\0';
		status = send(*comm_socket, buffer, count_read, 0);
		if(status != count_read){
			close_client_handling("Couldn't send the correct number of bytes", comm_socket);
			return ((void*)-1);
		}
	}
	if(count_read == -1){
			close_client_handling("Error while reading file", comm_socket);
			return ((void*)-1);
		}
		
	close_client_handling("File succesfully sent!. Closing communication with client ... \n", comm_socket);
	
	status = fclose(f);
	if (status == EOF){
		perror("fclose");
	}
	
	return NULL;
}

//function called by select when new incoming connection is received
int accept_connection(){
	unsigned int 		client_address_len;
	struct sockaddr_in	client_address;
	int			*comm_socket;
	pthread_t		handler_thread;
	
	//cannot pass local variable to thread, need to allocate on heap
	comm_socket = (int*)malloc(sizeof(int*));
	THE_RETURN(comm_socket == NULL, "Couldn't allocate memory for communication socket", -1);
	
	//accepting the connection
	client_address_len = sizeof(client_address);
	*comm_socket = accept(listen_socket, (struct sockaddr*) &client_address, &client_address_len);
	THE_RETURN(*comm_socket == -1, "Couldn't accept client connection", -1);
	
	printf("New connection accepted\n");
	
	//starting serving
	sem_wait(&thread_sem);
	pthread_create(&handler_thread, NULL, &send_file, (void*)(comm_socket));
	
	return 0;
}

int main(int argc, char *argv[]){
	fd_set 	listen_set;
	int		server_running = 1, status;
	
	THE_END(argc != 2, "Please only supply the server port");
	
	start_server(argv[1]);
	//main select loop
	while(server_running == 1){
		FD_ZERO(&listen_set);
		FD_SET(STDIN_FILENO, &listen_set);
		FD_SET(listen_socket, &listen_set);
		
		status = select(listen_socket + 1, &listen_set, NULL, NULL, NULL);
		if (status == -1){
			THE_END_CLEANUP(1 == 1, "Error while reading select", close_server);
		}
		else{
			if(FD_ISSET(0, &listen_set)){
				server_running = 0;
			}
			if(FD_ISSET(listen_socket, &listen_set)){
				printf("\nNew incoming connection\n");
				accept_connection();
			}
		}
	}
	
	close_server();
	
	return 0;
}
