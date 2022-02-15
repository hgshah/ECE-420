#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include "common.h"

// global variables
char** theArray;
int NUM_STR_;


void *ProcessRequest(void* rank) {
// shouldn't matter the threadID(rank) of the server threads

return 0;

// setContent, getContent, ParseMsg are proivded functions in common.h (should #include)
}


int main(int argc, char* argv[])
{


	long       thread;  /* Use long in case of a 64-bit system */
	pthread_t* thread_handles; 
	struct sockaddr_in sock_var_;

	
	
	
	/* Command line parameter count should be 4 */
	if (argc != 4){ 
		fprintf(stderr, "usage: %s <Size of theArray_ on server> <server ip> <server port>\n", argv[0]);
		exit(0);
	}
	NUM_STR_ = strtol(argv[1], NULL, 10);
	char *server_ip = argv[2];
	int server_port = strtol(argv[3], NULL, 10);
	
	
	
	
	
	
	pthread_mutex_t mutexes[NUM_STR_];
	theArray = (char**)malloc(NUM_STR_*sizeof(char*)); 
	
	// memoray allocate array of pointers to strings, and initialize an array of mutexes
	for (int i=0; i<NUM_STR_; i++) {
		pthread_mutex_init(&mutexes[i], NULL);
		theArray[i] = (char*)malloc(COM_BUFF_SIZE*sizeof(char)); //str_msg_default;
	}
	
	
	// populate char array
	char str_msg_default[COM_BUFF_SIZE];
	for (int i=0; i<NUM_STR_; i++) {
		sprintf(str_msg_default, "String %d: the initial value", i); 
		theArray[i] = str_msg_default; // seems to be working when tested
		// printf(theArray[i]);
	}
	
	
	
	thread_handles = malloc(COM_NUM_REQUEST*sizeof(pthread_t)); // 1000 threads
	
	/* Initialize socket address and port*/
	sock_var_.sin_addr.s_addr=inet_addr(server_ip);
	sock_var_.sin_port=server_port;
	sock_var_.sin_family=AF_INET;
	
	
	// pass char array to threads or make it global? pass mutex's to it???
	/* Create threads */
	for (thread = 0; thread < COM_NUM_REQUEST; thread++) {  
		pthread_create(&thread_handles[thread], NULL, 
			ProcessRequest, (void*) thread);  // server should open CON_NUM_REQUEST threads to handle concurrent client requests
	}
	/* Finalize threads */
	for (thread = 0; thread < COM_NUM_REQUEST; thread++) {
	    	pthread_join(thread_handles[thread], NULL); 
	}
	
	free(thread_handles);
	return 0;
}
