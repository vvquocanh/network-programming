#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define LOCALHOST "127.0.0.1"
#define SERVERPORT 9999
#define MYMSGLEN 2048

void mirror ( char * msg )
{
        int i ;
        int length ;
        char car ;
        
        length = strlen ( msg ) ;
        
        for ( i = 0 ; i < ( length / 2 ) ; i ++ )
        {
            car = msg [ i ] ;
            msg [ i ] = msg [ length - i - 1 ] ;
            msg [ length - i - 1 ] = car ;
        }
}

int create_socket() {
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	
	if (sock == -1) {
		perror("Couldn't create socket\n");
		exit(1);
	}
	
	printf("Socket created\n");
	
	return sock;
}

void set_socket_option(int sock) {
	int enable = 1;
	
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) != -1) return;
	
	perror("Fail to set socket option\n");
	close(sock);
	exit(1);
}

void setup_server(struct sockaddr_in *addr) {
	memset(addr, 0, sizeof(struct sockaddr_in));
	
	addr->sin_addr.s_addr = inet_addr(LOCALHOST);
	addr->sin_family = AF_INET;
	addr->sin_port = htons(SERVERPORT);
}

void bind_socket(int sock, struct sockaddr_in *addr) {
	if (bind(sock, (struct sockaddr*) addr, sizeof(struct sockaddr_in)) == 0) {
		printf("Bind done\n");
		return;
	};
	
	perror("Fail to bind socket\n");
	close(sock);
	exit(1);
}

int establish_connection() {
	int sock = create_socket();
	
	set_socket_option(sock);
	
	struct sockaddr_in server;
	setup_server(&server);
	
	bind_socket(sock, &server);
	
	return sock;
}

ssize_t receive_message(int sock, struct sockaddr_in *client, char client_message[], size_t client_message_size) {
	memset(client, 0, sizeof(struct sockaddr_in));
	socklen_t len = sizeof(*client);

	return recvfrom(sock, client_message, client_message_size, 0, (struct sockaddr *) client, &len);
}

void print_client_information(struct sockaddr_in *client, char client_message[]) {
	printf("The address of the client --> %s:%d with message: %s\n", 
		inet_ntoa(client->sin_addr), ntohs(client->sin_port), client_message);
}

void send_message(int sock, struct sockaddr_in *client, char client_message[]) {
	char server_reply[strlen(client_message) + 1];
	
	strcpy(server_reply, client_message);
	mirror(server_reply);
	
	socklen_t len = sizeof(*client);
	if (sendto(sock, server_reply, strlen(server_reply) + 1, 0, (struct sockaddr*) client, len) == -1) {
		printf("Fail to reply client\n");
	} 
}

int main(int argc, char *argv[]) {
	int sock = establish_connection();
	
	struct sockaddr_in client;
	char client_message[MYMSGLEN];
	
	while (1) {
		printf("\nListening for incoming messages...\n");
		
		ssize_t read_size = receive_message(sock, &client, client_message, sizeof(client_message));
		
		if (read_size == -1) {
			perror("Fail to receive message\n");
			continue;
		}
		
		client_message[read_size] = '\0';

		print_client_information(&client, client_message);
		
		send_message(sock, &client, client_message);
	}
	
	return 0;
}
