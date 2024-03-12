#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include "thread_allocation.h"

#define SERVERPORT 8888
#define MYMSGLEN 2048
#define MAXQUEUE 10

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

typedef struct {
	int client_sock;
	pthread_t tid;
	my_thread* thread_map;
} thread_args;	

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

void create_thread_map(my_thread* thread_map) {
	if (allocate_thread_map(thread_map, MAXQUEUE) != -1) return;
	
	perror("Fail to allocate thread map");
	exit(1);
}

int accept_connection(int server_sock, struct sockaddr_in *client) {
	socklen_t len = sizeof(struct sockaddr_in);
	int client_sock = accept(server_sock, (struct sockaddr *) client, &len);
	if (client_sock != -1) return client_sock;
	
	perror("Fail to accept client");
	return -1;
}

void print_socket_information(int sock, struct sockaddr_in *addr) {
	socklen_t len = sizeof(struct sockaddr_in);
	
	int code = getsockname(sock, (struct sockaddr*) addr, &len);
	if (code == -1) {
		perror("Fail to get local socket information");
		return;
	}
	printf("The local address bound to the current socket --> %s:%d \n", 
		inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
		
	code = getpeername(sock, (struct sockaddr*) addr, &len);
	if (code == -1) {
		perror("Fail to get peer socket information");
		return;
	}
	printf("The peer address bound to the peer socket --> %s:%d \n", 
		inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
}

int establish_connection(int server_sock) {
	printf("\nWaiting for incoming connections ....\n");
	
	struct sockaddr_in client;
	int client_sock = accept_connection(server_sock, &client);
	
	print_socket_information(client_sock, &client);
	
	return client_sock;
}

ssize_t receive_message(int client_sock, char client_message[], ssize_t message_size) {
	printf("Waiting for a string to process....\n" ) ;
	
	memset(client_message, 0, MYMSGLEN);
	 
	return recv(client_sock, client_message, message_size, 0);
}

int answer_message(int client_sock, char client_message[], ssize_t message_size) {
	char server_reply[message_size];
	
	strcpy(server_reply, client_message);
	
	mirror(server_reply);
	
	return send(client_sock, &server_reply, message_size, 0);
}

void* thread_response(void* new_args) {
	thread_args args = *((thread_args *) new_args);
	free(new_args);
		
	int client_sock = args.client_sock;
	pthread_t tid = args.tid;
	my_thread* thread_map = args.thread_map;
	
	char client_message[MYMSGLEN];
	
	while(1)
	{
	  	ssize_t read_size = receive_message(client_sock, client_message, sizeof(client_message));
	  	
		if (read_size == 0) {
			printf("Client disconnected\n");
			fflush(stdout);
			break;
		}
		else if (read_size == -1) {
			perror("Failed to receive from client\n");
			break;
		}
		
		client_message[read_size] = '\0';
		printf("Client message: %s\n", client_message);
		
		if (answer_message(client_sock, client_message, read_size) != -1) continue;
		
		perror("Fail to answer client");
		break;
	}
	
	release_thread(thread_map, MAXQUEUE, tid);
	pthread_exit((void *)0);
}

void response(int client_sock, my_thread* thread_map) {
	pthread_t tid = allocate_thread(thread_map, MAXQUEUE);
	if (tid == -1) {
		printf("There is no thread available at the moment");
		return;
	}
	
	thread_args* new_args = (thread_args *)malloc(sizeof(thread_args));
	new_args->client_sock = client_sock;
	new_args->tid = tid;
	new_args->thread_map = thread_map;
	
	if (pthread_create(&tid, NULL, thread_response, new_args) == 0) return;
	
	printf("Fail to create a thread");
	return;
}

int main(int argc, char *argv[]) {
	int server_sock = start_server();
	
	my_thread thread_map[MAXQUEUE];
	create_thread_map(thread_map);
	
	while (1) {
		
		int client_sock = establish_connection(server_sock);
		
		if (client_sock == -1) {
			perror("Fail to accept connection from client");
			continue;
		}
		
		response(client_sock, thread_map);
	}
	
	return 0;
}
