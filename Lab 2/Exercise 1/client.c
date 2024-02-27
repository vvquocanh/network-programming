#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MYMSGLEN 2048

int main(int argc, char* argv[]) {
	int sock;
	struct sockaddr_in server;
	char message[MYMSGLEN], server_reply[MYMSGLEN];
	
	sock = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sock == -1) {
		printf("Could not create socket\n");
		return -1;
	}
	
	printf("socket created\n");
	
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons(8888);

	int res;
	if ((res = connect(sock, (struct sockaddr*) &server, sizeof(server))) < 0) {
		perror("Connection failed error\n");
		close(sock);
		return -1;
	}
	
	printf("Return value: %d\n", res);
	
	printf("connection established, waiting to be accepted ......");
	
	while (1) {
		memset(message, 0, MYMSGLEN);
		printf("\nPlease type a message to transfer for processing:");
		scanf("%s", message);	
		
		if (send(sock, message, strlen(message), 0) < 0) {
			printf("send failed\n");
			close(sock);
			return -1;
		}
		
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
