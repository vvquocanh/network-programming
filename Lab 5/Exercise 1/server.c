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

void reset_interval(struct timeval *interval) {
	interval->tv_sec = TIMEOUT;
	interval->tv_usec = 0;
}

void setup_fdset(int server_sock, fd_set *fd_read, fd_set *fd_read_master, struct timeval *interval) {
	FD_ZERO(fd_read);
    	FD_ZERO(fd_read_master);
    	
    	FD_SET(server_sock, fd_read_master);
    	
    	reset_interval(interval);
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

int response(int client_sock, fd_set *fd_read_master) {
	char client_message[MYMSGLEN];
	memset(client_message, 0, MYMSGLEN);
	ssize_t read_size = recv(client_sock, client_message, sizeof(client_message), 0);
	
	if (read_size == 0) {
		printf("Client disconnected\n");
		fflush(stdout);
		FD_CLR(client_sock, fd_read_master);
	}
	else if (read_size == -1) {
		perror("Failed to receive from client\n");
		FD_CLR(client_sock, fd_read_master);
	}
		
	client_message[read_size] = '\0';
	printf("Client message: %s\n", client_message);
		
	if (send(client_sock, &client_message, sizeof(client_message), 0) == -1) 
		perror("Fail to answer client");
		
}

int main(int argc, char *argv[]) {
	int server_sock = start_server();
	
	fd_set fd_read, fd_read_master;
	struct timeval interval;
	setup_fdset(server_sock, &fd_read, &fd_read_master, &interval);
	
	int highest_sock_desc = server_sock;
	int sock_desc[MAXSOCKDESC] = {0};
	
	while (1) {
		
		fd_read = fd_read_master;
		
		int rv = select(highest_sock_desc + 1, &fd_read, NULL, NULL, &interval);
		
		if (rv == -1) {
			perror("Error on the select() function\n");
			exit(1);
		}
		else if (rv == 0) {
			printf("Time out! No data after %d seconds\n", TIMEOUT);
            		reset_interval(&interval);
		}
		else {
			if (FD_ISSET(server_sock, &fd_read)) {
				int client_sock = establish_connection(server_sock);
		
				if (client_sock == -1) {
					perror("Fail to accept connection from client\n");
					continue;
				}
				else 
					printf("New client arrived: %d\n", client_sock);
				
				for (int i = 0; i < MAXSOCKDESC; i++) {
					if (sock_desc[i] != 0) continue;
					
					sock_desc[i] = client_sock;
					break;
				}
				
				FD_SET(client_sock, &fd_read_master);
				if (client_sock > highest_sock_desc) highest_sock_desc = client_sock;
			}
			
			for (int i = 0; i < MAXSOCKDESC; i++) {
				if (sock_desc[i] == 0) continue;
				
				if (FD_ISSET(sock_desc[i], &fd_read)) {
					response(sock_desc[i], &fd_read_master);	
				} 
			}
		}
	}
	
	return 0;
}
