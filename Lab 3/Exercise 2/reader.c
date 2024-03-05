#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define LOCALHOST "127.0.0.1"
#define SERVERPORT 9999
#define CLIENTPORT 0
#define MYMSGLEN 2048

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

void setup_client(struct sockaddr_in *addr) {
	memset(addr, 0, sizeof(struct sockaddr_in));
	
	addr->sin_addr.s_addr = INADDR_ANY;
	addr->sin_family = AF_INET;
	addr->sin_port = htons(CLIENTPORT);
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

int establish_connection(struct sockaddr_in *server) {
	int sock = create_socket();
	
	set_socket_option(sock);
	
	setup_server(server);
	
	struct sockaddr_in client;
	setup_client(&client);
	
	bind_socket(sock, &client);
	
	return sock;
}

void send_to_server(int sock, struct sockaddr_in *server, char message[]) {
	socklen_t len = sizeof(*server);
	if (sendto(sock, message, strlen(message) + 1, 0, (struct sockaddr*) server, len) != -1) return;

	printf("Failed to send message to server\n");
	close(sock);
	exit(1);
}

void send_data(int sock, struct sockaddr_in *server) {
	char message[] = "JOIN#";
	
	send_to_server(sock, server, message);
}

void receive_from_server(int sock, struct sockaddr_in *server, char message[], size_t message_size) {
	socklen_t len = sizeof(*server);
	if (recvfrom(sock, message, message_size, 0, (struct sockaddr*) server, &len) != -1) return;
	
	printf("Failed to receive message to server\n");
	close(sock);
	exit(1);
}

void receive_data(int sock, struct sockaddr_in *server) {
	char server_message[MYMSGLEN];
	
	receive_from_server(sock, server, server_message, sizeof(server_message));
	
	printf("Writer's message: %s\n", server_message);
}

int main(int argc, char* argv[]) {
	struct sockaddr_in server;
	int sock = establish_connection(&server);
	
	send_data(sock, &server);
	
	while (1) {
		printf("\nWaiting for writer's message ...\n");
		receive_data(sock, &server);
	}
	
	return 0;
}
