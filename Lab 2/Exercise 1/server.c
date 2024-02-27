#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int palindrome ( char * s )
{
	char * t = s + strlen ( s ) - 1 ;

	while ( * s == * t )
		s ++, t -- ;
	return s >= t ;
}

int main(int argc, char *argv[]) {
	
	int socket_desc, client_sock;
	int socket_size, read_size;
	struct sockaddr_in server, client;
	
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	
	if (socket_desc == -1) {
		printf("Could not create socket\n");
		return -1;
	}
	
	printf("socket created\n");
	
	int code, enable = 1;
	code = setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
	if (code == -1) {
		perror("setsockopt");
		return 1;
	}
	
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_family = AF_INET;
	server.sin_port = htons(8888);
	
	if (bind(socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0) {
		//print the error message
		perror("bind failed. Error");
		close(socket_desc);
		return -1;
	}
	
	printf("bind done\n");
	
	code = listen(socket_desc, 3);
	
	if (code == -1) {
		perror("listen");
		exit(1);
	}
	
	printf("Waiting for incoming connections ....\n");
	
	socket_size = sizeof(struct sockaddr_in);
	
	client_sock = accept(socket_desc, (struct sockaddr *) &client, (socklen_t *) &socket_size);
	
	if (client_sock < 0) {
		close(socket_desc);
		perror("accept failed");
		return -1;
	}
	
	printf("Connection accepted\n");
	
	while (1);
	
	return 0;
}
