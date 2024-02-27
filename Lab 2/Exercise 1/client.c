#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char* argv[]) {
	int sock;
	struct sockaddr_in server;
	
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
	
	while (1);
	
	return 0;
}
