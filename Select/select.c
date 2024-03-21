#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

int main() {
    	int sock, s1, s2, fdmax, rv;
    	char buf[256];
    	struct timeval interval;
    	fd_set fdRead, fdReadMaster;

    	// Socket creation
    	sock = socket(AF_INET, SOCK_STREAM, 0);
    	// Binding and listening setup
    	
    	struct sockaddr_in server, client1, client2;
    	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_family = AF_INET;
	server.sin_port = htons(8888);
	
	int enable = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
	
    	bind(sock, (struct sockaddr *) &server, sizeof(server));
    	
    	listen(sock, 2);
	
	int socket_size = sizeof(struct sockaddr_in);
    	// Pretend we've two connected clients to the server that we want to watch
    	// and we need to execute some other processing every 3 seconds
    	s1 = accept(sock, (struct sockaddr *) &client1, (socklen_t *) &socket_size);
    	s2 = accept(sock, (struct sockaddr *) &client2, (socklen_t *) &socket_size);

    	// Set up the fdsets of file descriptors.
    	// Set fd_set to 0
    	FD_ZERO(&fdRead);
    	FD_ZERO(&fdReadMaster);

    	// Initialize fd_set with descriptors to monitor
    	FD_SET(s1, &fdReadMaster);
   	FD_SET(s2, &fdReadMaster);

    	// Initialize the timeout interval
    	interval.tv_sec = 3;
    	interval.tv_usec = 0;

    	while (1) {
        	fdmax = s2; // We suppose that s2 > s1
        	fdRead = fdReadMaster;

        	// Wait for events on the sockets, 3-second timeout
        	rv = select(fdmax + 1, &fdRead, NULL, NULL, &interval);
        	if (rv == -1) {
            		perror("select"); // error occurred in select()
            		close(sock);
            		close(s1);
            		close(s2);
            		exit(0);
        	} else if (rv == 0) {
            		printf("Timer Hello World: Timeout occurred! No data after 3 seconds.\n");
            		// do some processing here, and re init the timeout interval
            		interval.tv_sec = 3;
            		interval.tv_usec = 0;
        	} else {
        		memset(buf, 0, 256);
            		// check for events on s1:
            		if (FD_ISSET(s1, &fdRead)) {
                		// receive data from s1
                		recv(s1, buf, sizeof(buf), 0); // do some processing here
                		printf("From s1: %s\n", buf);
                		write(s1, buf, sizeof(buf));
			}
            		if (FD_ISSET(s2, &fdRead)) {
				// receive data from s2
                		recv(s2, buf, sizeof(buf), 0); // do some processing here
                		printf("From s2: %s\n", buf);
                		write(s2, buf, sizeof(buf));
                	}
        }
    }

    return 0;
}

