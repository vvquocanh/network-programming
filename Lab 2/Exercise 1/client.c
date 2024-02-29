#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MYMSGLEN 2048

int main(int argc, char* argv[]) {
	int sock;
	int socket_size;
	struct sockaddr_in server, client;
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
	
	socket_size = sizeof(struct sockaddr_in);
	
	if (getsockname(sock, (struct sockaddr *) &client, (socklen_t *) &socket_size) != 0) {
		perror("Failed to get socket port");
		close(sock);
		return -1;
	}
		
	printf("socket information:\n\t"
	"- address: %s\n\t"
	"- port: %d\n",
	inet_ntoa(client.sin_addr), ntohs(client.sin_port));
	
	printf("connection established, waiting to be accepted ......\n");
	
	while (1) {
	
		memset(message, 0, MYMSGLEN);
		printf("\nType a string to the server: ");
		scanf("%s", message);	
		
		if (strcmp(message, "quit#") == 0) {
			close(sock);
			printf("Disconnected from the server\n");
			return 0;
		}
		
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
		
		printf("Server reply: %s\n", server_reply);
	}
	
	return 0;
}
