#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVERPORT 8888
#define MYMSGLEN 2048
#define MAXQUEUE 3
#define TIMEOUT 30
#define MAXSOCKDESC 50

int create_socket() {
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sock == -1) {
		perror("Couldn't create socket\n");
		exit(1);
	}
	
	printf("Socket created\n");
	
	return sock;
}

void set_socket_option(int sock) {
	int enable = 1;
	
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
		perror("Fail to set socket option\n");
		close(sock);
		exit(1);
	}
	
	
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) == -1) {
		perror("Fail to set socket option\n");
		close(sock);
		exit(1);
	}
}

void setup_server(struct sockaddr_in *addr) {
	memset(addr, 0, sizeof(struct sockaddr_in));
	
	addr->sin_addr.s_addr = INADDR_ANY;
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

void listen_to_client(int sock) {
	if (listen(sock, MAXQUEUE) != -1) {
		printf("Connection accepted\n");
		return;
	}
	
	perror("Fail to listen to client");
	close(sock);
	exit(1);
}

int start_server() {
	int sock = create_socket();
	
	set_socket_option(sock);
	
	struct sockaddr_in server;
	setup_server(&server);
	
	bind_socket(sock, &server);
	
	listen_to_client(sock);
	
	return sock;
}

int accept_connection(int server_sock, struct sockaddr_in *client) {
	socklen_t len = sizeof(struct sockaddr_in);
	int client_sock = accept(server_sock, (struct sockaddr *) client, &len);
	if (client_sock != -1) return client_sock;
	
	perror("Fail to accept client");
	return -1;
}

int establish_connection(int server_sock) {

	struct sockaddr_in client;
	
	return accept_connection(server_sock, &client);
}

int response(struct pollfd *ufds) {
	char client_message[MYMSGLEN];
	memset(client_message, 0, MYMSGLEN);
	ssize_t read_size = recv(client_sock, client_message, sizeof(client_message), 0);
	
	if (read_size == 0) {
		printf("Client disconnected\n");
		fflush(stdout);
		ufds->fd = 0;
	}
	else if (read_size == -1) {
		perror("Failed to receive from client\n");
		ufds->fd = 0;
	}
		
	client_message[read_size] = '\0';
	printf("Client message: %s\n", client_message);
		
	if (send(client_sock, &client_message, sizeof(client_message), 0) == -1) 
		perror("Fail to answer client\n");
		
}

int main(int argc, char *argv[]) {
	int server_sock = start_server();
	
	struct pollfd ufds[MAXSOCKDESC + 1];
	ufds[0].fd = server_sock;
	ufds[0].events = POLLIN;
	
	while (1) {
		
		int rv = poll(ufds, MAXSOCKDESC + 1, TIMEOUT * 1000);
		
		if (rv == -1) {
			perror("Error on the poll() function\n");
			exit(1);
		}
		else if (rv == 0) {
			printf("Time out! No data after %d seconds\n", TIMEOUT);
		}
		else {
			if (ufds[0].revents & POLLIN) {
				int client_sock = establish_connection(server_sock);
		
				if (client_sock == -1) {
					perror("Fail to accept connection from client\n");
					continue;
				}
				else 
					printf("New client arrived: %d\n", client_sock);
				
				for (int i = 1; i < MAXSOCKDESC + 1; i++) {
					if (udfs[i].fd != 0) continue;
					
					ufds[i].fd = client_sock;
					ufds[i].events = POLLIN;
					break;
				}
			}
			
			for (int i = 1; i < MAXSOCKDESC + 1; i++) {
				if (udfs[i] == 0) continue;
				
				if (ufds[i].revents & POLLIN) {
					response(&ufds[i]);	
				} 
			}
		}
	}
	
	return 0;
}
