/* 
This is the client program.
*/

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MYMSGLEN 2000

int main(int argc, char *argv[]) {
	
	int sock;
	struct sockaddr_in server;
	char message[MYMSGLEN], server_reply[MYMSGLEN];
	
	/*
	Creates a socket and the return value which is a file descriptor will be stored in the variable sock.
	
	The socket is created with 3 parameters:
	- Domain: AF_INET. It's the IPv4 protocols.
	- Type: SOCK_STREAM. It's the connected communication with end to end control flow.
	- Protocol: 0. It's the default protocol for this socket type.
	
	The return value equal -1 means there is an error on creating socket.	
	*/
	sock = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sock == -1) {
		printf("Could not create socket\n");
		return -1;
	}
	
	printf("socket created\n");
	
	/*
	Initializes the IPv4 server socket address structure.
	We assign:
	- IP address: 127.0.0.1
	- Address family: AF_INET (IPv4)
	- Port: 8888
	*/
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons(8888);
	
	/*
	Establishes a connection to a server by using:
	- Client socket
	- Server socket address
	- Size of socket address
	
	If the connection failed, close the socket.
	*/
	int res;
	if ((res = connect(sock, (struct sockaddr*) &server, sizeof(server))) < 0) {
		perror("Connection failed error\n");
		close(sock);
		return -1;
	}
	
	printf("Return value: %d\n", res);
	printf("connection established, waiting to be accepted ......");
	
	/*
	Creates a while loop to wait and receive input data from the users, send the data to the server and receive responses.
	*/
	while(1) {
		memset(message, 0, MYMSGLEN);
		printf("\nPlease type a message to transfer for processing:");
		scanf("%s", message);	
		
		/*
		Sends input data received from the users to the socket of the server.
		
		If the sending failed, close the socket.
		*/
		if (send(sock, message, strlen(message), 0) < 0) {
			printf("send failed\n");
			close(sock);
			return -1;
		}
		
		/*
		Receives responses from the socket of the server.
		
		If the receiving failed, close the socket.
		
		Otherwise, print server responses.
		*/
		memset(server_reply, 0, MYMSGLEN);
		if (recv(sock, server_reply, MYMSGLEN, 0) < 0) {
			printf("recv failed\n");
			close(sock);
			return -1;
		}
		
		printf("Server reply: %s", server_reply);
	}
	return 0;
}
