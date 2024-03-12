#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define THREADNUMBER 2

void *thread_routine() {
	pid_t tid = gettid();
	
	printf("\nProcess %d, thread: %ld: Hello World!\n", getpid(), (long)tid);
	
	sleep(5);
	
	printf("\nThread exit!\n");
	
	pthread_exit(NULL);
}

int main ()
{
 	pthread_t tid[THREADNUMBER];
 	
 	for (int i = 1; i <= 2; i++) {
 		int result = pthread_create(&(tid[i]), NULL, &thread_routine, NULL);
 		
 		if (result == 0) printf("\nThread %d created ...\n", i);
 		else printf("Cannot create thread: %s \n", strerror(result));
 		
 		sleep(2);
 	}
 	
 	sleep(10);
 	
 	printf("Main program exit\n");
 	
 	return 0;
}
