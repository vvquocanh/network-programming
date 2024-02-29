#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MYMSGLEN 2048

int palindrome ( char * s )
{
	char * t = s + strlen ( s ) - 1 ;

	while ( * s == * t )
		s ++, t -- ;
	return s >= t ;
}

typedef struct Response {
	char client_message[MYMSGLEN];
	unsigned int character_number;
	float cost;
	char* reply_message;
	
} response;

float calculate_cost(unsigned int character_number) {
	return (float)character_number * 0.23;
}

response generate_response(char client_message[MYMSGLEN]) {
	response response_message;
	
	strncpy(response_message.client_message, client_message, MYMSGLEN);
	unsigned int character_number = strlen(client_message);
	response_message.character_number = character_number;
	response_message.reply_message = palindrome(client_message) ? "The string is palindrome" : "The string is NOT palindrome";
	response_message.cost = calculate_cost(character_number);
	
	return response_message;
}

int main(int argc, char *argv[]) {
	
	int socket_desc, client_sock;
	int socket_size, read_size;
	struct sockaddr_in server, client;
	char client_message[MYMSGLEN];
	
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
	
	printf("Connection accepted\n");
	
	while (1) {
		printf("\nWaiting for incoming connections ....\n");
	
		socket_size = sizeof(struct sockaddr_in);
	
		client_sock = accept(socket_desc, (struct sockaddr *) &client, (socklen_t *) &socket_size);
		
		if (getpeername(client_sock, (struct sockaddr *) &client, (socklen_t *) &socket_size) != 0) {
			perror("Failed to get user's socket name");
			continue;
		}
		
		printf("A new client is connected with:\n\t"
		"- Client address: %d\n\t"
		"- Client port: %d\n",
		client.sin_addr.s_addr, client.sin_port);
		
		if (client_sock < 0) {
			close(socket_desc);
			perror("accept failed");
			continue;
		}
		
		while ((read_size = recv(client_sock, client_message, MYMSGLEN, 0)) > 0) {
		
			client_message[read_size] = '\0';
			printf("Msg received: %s, size of the message received, %d\n", client_message, read_size);
			
			response response_message = generate_response(client_message);
			
			write(client_sock, (char *) &response_message, sizeof(response_message));
		} 
		
		if (read_size == 0) {
			printf("client disconnected\n");
			fflush(stdout);
		}
		
		else if (read_size == -1) {
			printf("recv failed\n");
		}
	}

	
	return 0;
}
