#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define THREADNUMBER 10

void *thread_routine(void *rank) {
	sleep(1);
	
	pid_t tid = gettid();
	
	printf("\nProcess %d, thread: %ld, rank: %d: Hello World!\n", getpid(), (long)tid, *((int *)rank));
		
	printf("\nThread exit!\n");
	
	pthread_exit(NULL);
}

int main ()
{
 	pthread_t tid[THREADNUMBER];
 	
 	for (int i = 0; i < THREADNUMBER; i++) {
 		
 		int *new_value = malloc(sizeof(*new_value));
 		*new_value = i;
 		
 		int result = pthread_create(&(tid[i]), NULL, &thread_routine, new_value);
 		
 		if (result == 0) printf("\nThread %d created ...\n", i);
 		else printf("Cannot create thread: %s \n", strerror(result));
 	}
 	
 	for (int i = 0; i < THREADNUMBER; i++) {
 		pthread_join(tid[i], NULL);
 	}
 	
 	printf("Main program exit\n");
 	
 	return 0;
}
