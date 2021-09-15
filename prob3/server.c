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

#define QUEUE_SIZE_LIMIT 100

int 	listen_socket;
sem_t	thread_sem;

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

void start_server(char *server_port_input){

	int 			status, server_port;
	struct sockaddr_in	listen_address;
	
	server_port = strtol(server_port_input, NULL, 10);
	THE_END(server_port == 0, "Invalid server port");
	
	status = sem_init(&thread_sem, 0, MAX_CLIENTS);
	THE_END(status == -1, "Cannot initialize semaphore");

	printf("Starting the server\n");
	listen_socket = socket(PF_INET, SOCK_STREAM, 0);
	THE_END(listen_socket == -1, "Cannot create listening socket");
	
	listen_address.sin_family = AF_INET;
	listen_address.sin_addr.s_addr = htonl(INADDR_ANY);
	listen_address.sin_port = htons(server_port);
	
	status = bind(listen_socket, (struct sockaddr *) &listen_address, sizeof(listen_address));
	THE_END_CLEANUP(status == -1, "Couldn't bind listening socket", close_server);
	
	status = listen(listen_socket, QUEUE_SIZE_LIMIT);
	THE_END_CLEANUP(status == -1, "Couldn't start listening", close_server);
	printf("Listening for new connections. Press any key to close the server\n");
}

void close_client_handling(char *ret_msg, int comm_socket){
	printf("%s", ret_msg);
	sem_post(&thread_sem);
	close(comm_socket);
}

void *send_file(void *arg){
	int 		status, count_read, comm_socket;
	char		file_name[BUFFER_SIZE];
	char		fnf[] = "FILE NOT FOUND!";
	char		buffer[BUFFER_SIZE];
	FILE		*f;
	
	comm_socket = *((int*)arg);
	
	status = recv(comm_socket, file_name, BUFFER_SIZE, 0);
	if(status == -1){
		close_client_handling("Couldn't receive file name from client", comm_socket);
		return ((void*)-1);
	}
	
	file_name[status] = '\0';
	
	printf("Looking for file %s ...\n", file_name);
	f = fopen(file_name, "rb");
	if (f == NULL){
		status = send(comm_socket, fnf, strlen(fnf), 0);
		close_client_handling("File not found! Closing connection!\n", comm_socket);
		return ((void*)-1);
	}
	
	printf("Found file %s. Sending confirmation ...\n", file_name);	
	status = send(comm_socket, "OK", 2, 0);
	if(status == -1){
		close_client_handling("Couldn't send confirmation to client", comm_socket);
		return ((void*)-1);
	}
	
	printf("Sending file %s\n", file_name);
	while((count_read = fread(buffer, 1, BUFFER_SIZE, f)) > 0){
		buffer[count_read] = '\0';
		status = send(comm_socket, buffer, count_read, 0);
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

int accept_connection(){
	unsigned int 		client_address_len;
	struct sockaddr_in	client_address;
	int			comm_socket;
	pthread_t		handler_thread;
	
	client_address_len = sizeof(client_address);
	comm_socket = accept(listen_socket, (struct sockaddr*) &client_address, &client_address_len);
	THE_RETURN(comm_socket == -1, "Couldn't accept client connection", -1);
	
	printf("New connection accepted\n");
	
	sem_wait(&thread_sem);
	pthread_create(&handler_thread, NULL, &send_file, (void*)(&comm_socket));
	
	return 0;
}

int main(int argc, char *argv[]){
	fd_set 	listen_set;
	int		server_running = 1, status;
	
	THE_END(argc != 2, "Please only supply the server port");
	
	start_server(argv[1]);
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
