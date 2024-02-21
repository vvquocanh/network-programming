/* 
This is the server program.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> //write

#define MYMSGLEN 2000

int main(int argc, char *argv[]) {
	
	int socket_desc, client_sock;
	int socket_size, read_size;
	struct sockaddr_in server, client;
	char client_message[MYMSGLEN];
	
	/*
	Creates a socket and the return value which is a file descriptor will be stored in the variable sock_desc.
	
	The socket is created with 3 parameters:
	- Domain: AF_INET. It's the IPv4 protocols.
	- Type: SOCK_STREAM. It's the connected communication with end to end control flow.
	- Protocol: 0. It's the default protocol for this socket type.
	
	The return value equal -1 means there is an error on creating socket.	
	*/
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	
	if (socket_desc == -1) {
		printf("Could not create socket\n");
		return -1;
	}
	
	printf("socket created\n");
	
	/*
	Sets the socket option with:
	- Level: SOL_SOCKET. This is the level that is interpreted by the socket level.
	- Option: SO_REUSEADDR. This option allows other sockets to bind() to this port.
	
	The return value The return value equal -1 means there is an error on seting socket options.
	*/
	int code, enable = 1;
	code = setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
	if (code == -1) {
		perror("setsockopt");
		return 1;
	}
	
	/*
	Initializes the IPv4 server socket address structure.
	We assign:
	- IP address: INADDR_ANY. This means binding the socket to all interfaces.
	- Address family: AF_INET (IPv4)
	- Port: 8888
	*/
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_family = AF_INET;
	server.sin_port = htons(8888);
	
	/*
	Assign the speficied server socket address to the created socket by the file descriptor socket_desc.
	
	If binding failed, close the socket.
	*/
	if (bind(socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0) {
		//print the error message
		perror("bind failed. Error");
		close(socket_desc);
		return -1;
	}
	
	printf("bind done\n");
	
	/*
	Listens for connections with the file descriptor socket_desc and 3 active participants that can "wait" for a connection.
	
	If listening failed, exit.
	*/
	code = listen(socket_desc, 3);
	
	if (code == -1) {
		perror("listen");
		exit(1);
	}
	
	/*
	Extracts the first connection request on the queue of pending connections for the listening socket.
	
	Stores the client socket to the client_sock variable.
	
	If failed, close the server socket.
	*/
	printf("Waiting for incoming connections ....\n");
	socket_size = sizeof(struct sockaddr_in);
	
	client_sock = accept(socket_desc, (struct sockaddr *) &client, (socklen_t *) &socket_size);
	
	if (client_sock < 0) {
		close(socket_desc);
		perror("accept failed");
		return -1;
	}
	
	printf("Connection accepted\n");
	
	/*
	Creates a while loop to receive, print, and reply messages to the client socket.
	
	Divdes client messages into short segments.
	*/
	while ((read_size = recv(client_sock, client_message, MYMSGLEN, 0)) > 0) {
		client_message[read_size] = '\0';
		printf("Msg received: %s, size of the message received, %d\n", client_message, read_size);
		
		/*
		Writes the message to the client socket to send back the response to the client.
		*/
		write(client_sock, client_message, read_size);
	} 
	
	if (read_size == 0) {
		printf("client disconnected\n");
		fflush(stdout);
	}
	else if (read_size == -1) {
		printf("recv failed\n");
	}
	
	close(socket_desc);
	close(client_sock);
	
	return 0;
}
