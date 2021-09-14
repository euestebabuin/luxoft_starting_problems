#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>

#include "socket_api.h"

int client_socket;

void close_socket(){
	int 	status;
	
	printf("Closing client ...\n");
	status = close(client_socket);
	THE_END(status == -1, "Cannot close socket");	
}

void open_socket_connect(int server_port){

	struct sockaddr_in	server_address;
	int			status;

	printf("Opening client ...\n");
	client_socket = socket(PF_INET, SOCK_STREAM, 0);
	THE_END(client_socket == -1, "Cannot create socket");
	
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_address.sin_port = htons(server_port);
	
	status = connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address));
	THE_END_CLEANUP(status == -1, "Cannot connect to server", close_socket);
}

int receive_file(int client_socket, char *file_name){
	FILE		*f;
	
	
	
	return 0;	
}

int main(int argc, char *argv[]){
	int 			status, server_port;
	char			buffer[BUFFER_SIZE];
	
	THE_END(argc != 3, "Please only supply the server port and the name of the required file!\n \
	./client_ex <server_port> <file_name>");
	
	server_port = strtol(argv[1], NULL, 10);
	THE_END(server_port == 0, "Invalid server port");
	
	open_socket_connect(server_port);
	
	printf("Sending request to server ...\n");
	status = send(client_socket, argv[2], strlen(argv[2]), 0);
	THE_END_CLEANUP(status != strlen(argv[2]), "Sent a different number of bytes than expected", close_socket);
	
	status = recv(client_socket, buffer, BUFFER_SIZE, 0);
	THE_END_CLEANUP(status == -1, "Couldn't receive response from server!", close_socket);

	if(buffer[0] == 'O' && buffer[1] == 'K'){
		receive_file(client_socket, argv[2]);
	}
	else{
		printf("FILE NOT FOUND\n");
	}
	
	close_socket();
	
	return 0;
}
