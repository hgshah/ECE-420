#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include "common.h"
#include "timer.h"

typedef struct {
    int clientFileDescriptor; 
    int requestNumber;
    pthread_mutex_t *mutex_pointer;  
} RequestParameters; 
char** theArray;
double access_times[COM_NUM_REQUEST];

void *ProcessRequest(void* args) {
	// retrieve struct passed to thread function
	RequestParameters *param_struct = (RequestParameters*) args;
	int clientFileDescriptor = param_struct->clientFileDescriptor;
	pthread_mutex_t *mutex_array_pointer = param_struct->mutex_pointer;

	char incoming_msg[COM_BUFF_SIZE];
	double start_time, end_time;
	ClientRequest *rqst_struct = (ClientRequest*) malloc(sizeof(ClientRequest));

	// retrive, display and parse incoming message 
	read(clientFileDescriptor,incoming_msg,COM_BUFF_SIZE);
	//printf("reading client request:  %s\n",incoming_msg);
	
	GET_TIME(start_time); // start timing here
	ParseMsg(incoming_msg, rqst_struct);

	if (rqst_struct->is_read == 0) { // write request
		pthread_mutex_lock(&mutex_array_pointer[rqst_struct->pos]);
		
		// update value in memory 
		setContent(rqst_struct->msg, rqst_struct->pos, theArray); 
	
		// retrieve and return updated value to client
		char retrieved_string[COM_BUFF_SIZE]; 
		getContent(retrieved_string, rqst_struct->pos, theArray); 
		GET_TIME(end_time); // end timing here
		write(clientFileDescriptor,retrieved_string,COM_BUFF_SIZE);
		
		pthread_mutex_unlock(&mutex_array_pointer[rqst_struct->pos]);
		//printf("Write request from client processed \n\n");	
	} else { // read request
		pthread_mutex_lock(&mutex_array_pointer[rqst_struct->pos]);

		// read value from memory and return it to client
		char retrieved_string[COM_BUFF_SIZE]; 
		getContent(retrieved_string, rqst_struct->pos, theArray); 
		GET_TIME(end_time); // end timing here
		write(clientFileDescriptor,retrieved_string,COM_BUFF_SIZE); 

		pthread_mutex_unlock(&mutex_array_pointer[rqst_struct->pos]);		
		//printf("Read request from client processed \n\n");
	}
	
	access_times[param_struct->requestNumber] = start_time - end_time;
	close(clientFileDescriptor);
	free(args);
	return 0;
}


int main (int argc, char* argv[]) {	
	/* Command line parameter count should be 4 */
	if (argc != 4) { 
		fprintf(stderr, "usage: %s <Size of theArray_ on server> <server ip> <server port>\n", argv[0]);
		exit(0);
	}
	
	int array_size = strtol(argv[1], NULL, 10);
	char *server_ip = argv[2];
	int server_port = strtol(argv[3], NULL, 10);
	
	long thread;  /* Use long in case of a 64-bit system */
	pthread_t* thread_handles = malloc(COM_NUM_REQUEST*sizeof(pthread_t));;
	pthread_mutex_t mutexes[array_size];
	theArray = (char**)malloc(array_size*sizeof(char*)); 
	
	// malloc array of pointers to strings, and initialize array of mutexes
	for (int i=0; i<array_size; i++) {
		pthread_mutex_init(&mutexes[i], NULL);
		theArray[i] = (char*)malloc(COM_BUFF_SIZE*sizeof(char)); //str_msg_default;
	}
	
	char str_msg_default[COM_BUFF_SIZE];
	
	// populate char array with default values
	for (int i=0; i<array_size; i++) {
		sprintf(str_msg_default, "String %d: the initial value", i); 
		theArray[i] = str_msg_default;
	}
	
	// create a socket
	int serverFileDescriptor=socket(AF_INET,SOCK_STREAM,0);
	int clientFileDescriptor;
	struct sockaddr_in sock_var_;
	
	// Initialize socket address and port
	sock_var_.sin_addr.s_addr=inet_addr(server_ip);
	sock_var_.sin_port=server_port;
	sock_var_.sin_family=AF_INET;
	
	if (bind(serverFileDescriptor,(struct sockaddr*)&sock_var_,sizeof(sock_var_))>=0) {
        	printf("socket has been created\n");
        	listen(serverFileDescriptor,2000); 
        	
        	while (1) {
			/* Create threads */
			for (thread = 0; thread < COM_NUM_REQUEST; thread++) {  
				clientFileDescriptor=accept(serverFileDescriptor,NULL,NULL);
				printf("Connected to client %d\n",clientFileDescriptor);
				
				// create parameters struct to pass to thread function
				RequestParameters *param_struct = (RequestParameters*) malloc(sizeof(RequestParameters));
				param_struct->clientFileDescriptor = clientFileDescriptor;
				param_struct->mutex_pointer = &mutexes[0];
				param_struct->requestNumber = thread;

				pthread_create(&thread_handles[thread], NULL, ProcessRequest, (void*) param_struct);  
			}
			
			/* Finalize threads */
			for (thread = 0; thread < COM_NUM_REQUEST; thread++) {
			    	pthread_join(thread_handles[thread], NULL); 
			}
			
			saveTimes(access_times, COM_NUM_REQUEST);
        	}
        	
        	close(serverFileDescriptor);
    	} else {
		printf("socket creation failed\n");
	}
	
	free(thread_handles);
	return 0;
}
