#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MYMSGLEN 2000

int main(int argc, char *argv[]) {
	int socket_desc;
	struct sockaddr_in server;
	char *message, server_reply[MYMSGLEN];
	struct hostent *he;
	
	struct in_addr inaddr;
	
	// Create socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1) {
		printf("Could not create socket\n");
		return -1;
	}
	
	// Get the website's IP address
	if ((he = gethostbyname("www.ordinateur.cc")) == NULL) {
		perror("Failed to get host's IP address\n");
		return 2;
	}
	
	memcpy((char *) &inaddr, he->h_addr, sizeof(struct in_addr));
	
	printf("IP address for httpd.apache.org is %s \n", inet_ntoa(inaddr));
	
	// Init server socket address
	server.sin_addr.s_addr = inaddr.s_addr;
	server.sin_family = AF_INET;
	server.sin_port = htons(80);
	
	// Connect to remote server
	if (connect(socket_desc, (struct sockaddr*) &server, sizeof(server)) < 0) {
		printf("connect error");
		return 1;
	}
	
	printf("Connected\n");
	
	// Send data
	message = "GET / HTTP/1.1\r\nhost: www.ordinateur.cc \r\n\r\n";
	
	if (send(socket_desc, message, strlen(message), 0) < 0) {
		puts("Send failed");
		return -1;
	}
	puts("message request sent");
	
	int res;
	memset(server_reply, 0, MYMSGLEN);
	
	while ((res = recv(socket_desc, server_reply, MYMSGLEN, 0)) != 0) {
		if (res < 0) {
			puts("recv failed");
			break;
		}
		else {
			printf("%s", server_reply);
			memset(server_reply, 0, MYMSGLEN);
		}
	}
	
	close(socket_desc);
	
	return 0;
}
