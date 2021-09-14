#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>


#include "socket_api.h"

#define QUEUE_SIZE_LIMIT 100

int listen_socket;

void close_server(){
	int 	status;
	
	printf("\nClosing the server\n");
	status = close(listen_socket);
	THE_END(status == -1, "Cannot close listening socket");	
}

void start_server(int server_port){

	int 			status;
	struct sockaddr_in	listen_address;

	printf("Starting the server\n\n");
	listen_socket = socket(PF_INET, SOCK_STREAM, 0);
	THE_END(listen_socket == -1, "Cannot create listening socket");
	
	listen_address.sin_family = AF_INET;
	listen_address.sin_addr.s_addr = htonl(INADDR_ANY);
	listen_address.sin_port = htons(server_port);
	
	status = bind(listen_socket, (struct sockaddr *) &listen_address, sizeof(listen_address));
	THE_END_CLEANUP(status == -1, "Couldn't bind listening socket", close_server);
	
	status = listen(listen_socket, QUEUE_SIZE_LIMIT);
	THE_END_CLEANUP(status == -1, "Couldn't start listening", close_server);
	printf("Listening for new connections ...\n");
}

int send_file(int comm_socket){
	int 		status;
	char		file_name[BUFFER_SIZE];
	char		fnf[] = "FILE NOT FOUND!";
	char		buffer[BUFFER_SIZE];
	FILE		*f;
	
	status = recv(comm_socket, file_name, BUFFER_SIZE, 0);
	THE_RETURN(status == -1, "Couldn't receive file name from client", -1);
	file_name[status] = '\0';
	
	printf("Looking for file %s ...\n", file_name);
	f = fopen(file_name, "rb");
	if (f == NULL){
		printf("File %s not found!\n", file_name);
		status = send(comm_socket, fnf, strlen(fnf), 0);
		THE_RETURN(status != strlen(fnf), "Sent a different number of bytes than expected", -1);
	}
	
	printf("Found file %s. Sending confirmation ...\n", file_name);	
	status = send(comm_socket, "OK", 2, 0);
	THE_RETURN(status == -1, "Couldn't send confirmation to client", -1);
	
	printf("Sending file %s\n", file_name);
	
	status = fread(buffer, 1, BUFFER_SIZE, f);
	
	status = close(comm_socket);
	THE_RETURN(status == -1, "Couldn't close communication socket", -1);
	
	fclose(f);
	
	return 0;
}

int main(int argc, char *argv[]){
	int 			comm_socket, server_port;
	unsigned int 		client_address_len;
	struct sockaddr_in	client_address;
	
	THE_END(argc != 2, "Please only supply the server port");
	
	server_port = strtol(argv[1], NULL, 10);
	THE_END(server_port == 0, "Invalid server port");
	
	start_server(server_port);
	
	client_address_len = sizeof(client_address);
	comm_socket = accept(listen_socket, (struct sockaddr*) &client_address, &client_address_len);
	THE_END_CLEANUP(comm_socket == -1, "Couldn't accept client connection", close_server);
	
	printf("\nNew connection accepted\n");
	
	send_file(comm_socket);
	
	close_server();
	
	return 0;
}
