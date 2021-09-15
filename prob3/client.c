#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "socket_api.h"

/*
* First read server documentation!!
*/

int client_socket;

//function which emulates network wait times
void sleep_milis(int milis){
	struct timespec tsm;
	
	tsm.tv_sec = milis / 1000;
	tsm.tv_nsec = 0;
	
	nanosleep(&tsm, &tsm);
}

void close_socket(){
	int 	status;
	
	printf("Closing client ...\n");
	status = close(client_socket);
	THE_END(status == -1, "Cannot close socket");	
}

// function which handles opening a connection by creating a socket and connecting via localhost to the server
void open_socket_connect(int server_port){

	struct sockaddr_in	server_address;
	int			status;

	//creating socket
	printf("Opening client endpoint...\n");
	client_socket = socket(PF_INET, SOCK_STREAM, 0);
	THE_END(client_socket == -1, "Cannot create socket");
	
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_address.sin_port = htons(server_port);
	
	//the connect() call
	status = connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address));
	THE_END_CLEANUP(status == -1, "Cannot connect to server", close_socket);
	printf("Succesfully connected to the server!\n");
}

//function which handles re3ceiving the file from server
int receive_file(int client_socket, char *file_name){
	FILE		*f;
	char		buffer[BUFFER_SIZE];
	int		count_received, status;
	
	printf("Receiving file ...\n");
	
	f = fopen(file_name, "wb");	
	THE_RETURN(f == NULL, "Couldn't create local file to be received", -1);
	
	// as on the server side, only count_received (not BUFFER_SIZE) bytes are written to the file
	while((count_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0){
		sleep_milis(500);
		buffer[count_received] = '\0';
		status = fwrite(buffer, 1, count_received, f);
		THE_RETURN(status != count_received, "Error while writing to local file", -1);
	}
	THE_RETURN(count_received == -1, "Error while receiving file", -1);
	
	fclose(f);
	printf("File successfully received!\n");
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
	
	//instructions which check with the server file availability
	printf("Sending request to server ...\n");
	status = send(client_socket, argv[2], strlen(argv[2]), 0);
	THE_END_CLEANUP(status != strlen(argv[2]), "Sent a different number of bytes than expected", close_socket);
	//onky receiving the first 2 bytes because at the moment we need to check if server sent the 'OK'. Reading more bytes might cause
	// the faulty reading of the file, since the server doesn't wait for any additional reconfirmation (SYN-ACK)
	status = recv(client_socket, buffer, 2, 0);
	THE_END_CLEANUP(status == -1, "Couldn't receive response from server!", close_socket);
	printf("Request granted\n");

	// the server sent OK as confirmation
	if(buffer[0] == 'O' && buffer[1] == 'K'){
		receive_file(client_socket, argv[2]);
	}
	else{
		printf("FILE NOT FOUND\n");
	}
	
	close_socket();
	
	return 0;
}
