#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#define SERVERPORT 8888
#define MYMSGLEN 2048
#define LOCALHOST "127.0.0.1"

int create_socket() {
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sock == -1) {
		perror("Couldn't create socket\n");
		exit(1);
	}
	
	printf("Socket created\n");
	
	return sock;
}

void setup_server(struct sockaddr_in *addr) {
	memset(addr, 0, sizeof(struct sockaddr_in));
	
	addr->sin_addr.s_addr = inet_addr(LOCALHOST);
	addr->sin_family = AF_INET;
	addr->sin_port = htons(SERVERPORT);
	
}

void connect_to_server(struct sockaddr_in *addr, int sock) {
	int code = connect(sock, (struct sockaddr*) addr, sizeof(struct sockaddr_in));
	if (code == -1) {
		perror("Couldn't connect to the server\n");
		close(sock);
		exit(1);
	}
	
	printf("Connection established, waiting to be accepted ......\n");
}

void print_socket_information(int sock) {
	struct sockaddr_in addr;
	socklen_t len = sizeof(struct sockaddr_in);
	
	int code = getsockname(sock, (struct sockaddr*) &addr, &len);
	if (code == -1) {
		perror("Fail to get local socket information");
		close(sock);
		exit(1);
	}
	printf("The local address bound to the current socket --> %s:%d \n", 
		inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
		
	code = getpeername(sock, (struct sockaddr*) &addr, &len);
	if (code == -1) {
		perror("Fail to get peer socket information");
		close(sock);
		exit(1);
	}
	printf("The peer address bound to the peer socket --> %s:%d \n", 
		inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
}

int connecting() {
	int sock = create_socket();
	
	struct sockaddr_in server;
	setup_server(&server);
	
	connect_to_server(&server, sock);
	
	print_socket_information(sock);
	
	return sock;
}

void set_user_input(char message[]) {
	memset(message, 0, MYMSGLEN);
	printf("\nType a string to the server: ");
	scanf("%s", message);
}

void process_user_input(int sock, char message[]) {
	if (strcmp(message, "quit#") != 0) return;
	
	close(sock);
	printf("Disconnected from the server\n");
	exit(1);
}

void send_to_server(int sock, char message[], size_t message_size) {
	if (send(sock, message, message_size, 0) != -1) return;
			
	printf("Send failed\n");
	close(sock);
	exit(1);
}

void send_data(int sock) {
	char message[MYMSGLEN];
	
	set_user_input(message);	
		
	process_user_input(sock, message);
	
	send_to_server(sock, message, strlen(message));
}

void receive_from_server(int sock, char server_reply[]) {
	if (recv(sock, server_reply, MYMSGLEN, 0) != -1) return;
	
	printf("Receive failed\n");
	close(sock);
	exit(1);
}

void receive_data(int sock) {
	char server_reply[MYMSGLEN];
	
	receive_from_server(sock, server_reply);
	
	printf("Mirror message: %s\n", server_reply);
}

int main(int argc, char* argv[]) {
	int sock = connecting();
	
	while (1) {
		send_data(sock);
		
		receive_data(sock);
	}
	
	return 0;
}
