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

typedef struct{
    int clientFileDescriptor; 
    pthread_mutex_t *mutex_pointer;  
} RequestParameters; // To store the parsed client message


void *ProcessRequest(void* args) {
	// retrieve array of mutexes passed to thread function
	//pthread_mutex_t *mutex_array_pointer = (pthread_mutex_t *) args;
	
	// HERE
	//int clientFileDescriptor = (int)args;
	RequestParameters *param_struct = (RequestParameters*) args;
	int clientFileDescriptor = param_struct->clientFileDescriptor;
	pthread_mutex_t *mutex_array_pointer = param_struct->mutex_pointer;



	char incoming_msg[COM_BUFF_SIZE];
	ClientRequest *rqst_struct = (ClientRequest*) malloc(sizeof(ClientRequest));

	// retrive and display incoming message 
	read(clientFileDescriptor,incoming_msg,COM_BUFF_SIZE);
	printf("reading client request:  %s\n",incoming_msg);

	// parse message
	ParseMsg(incoming_msg, rqst_struct);


	// process request
	if (rqst_struct->is_read == 0) { // write request
	
		// lock mutex
		pthread_mutex_lock(&mutex_array_pointer[rqst_struct->pos]);
		
		// update value in memory // get content/setcontent
		setContent(rqst_struct->msg, rqst_struct->pos, theArray); // &theArray????
	
		// retireve and return updated value to client
		char retrieved_string[COM_BUFF_SIZE]; //COM_BUFF_SIZE???
		getContent(retrieved_string, rqst_struct->pos, theArray); 
		write(clientFileDescriptor,retrieved_string,COM_BUFF_SIZE);//COM_BUFF_SIZE??? 
		
		// unlock mutex
		pthread_mutex_unlock(&mutex_array_pointer[rqst_struct->pos]);
		
		printf("Write Request processed from client\n\n");
		
	} else { // read request
	
		// lock mutex
		pthread_mutex_lock(&mutex_array_pointer[rqst_struct->pos]);

		// read value from memory
		char retrieved_string[COM_BUFF_SIZE]; //COM_BUFF_SIZE???
		getContent(retrieved_string, rqst_struct->pos, theArray); 
		
		// return value to client
		write(clientFileDescriptor,retrieved_string,COM_BUFF_SIZE);//COM_BUFF_SIZE??? 

		// unlock mutex
		pthread_mutex_unlock(&mutex_array_pointer[rqst_struct->pos]);
		
		printf("Read Request processed\n\n");
	}
	
	close(clientFileDescriptor);
	return 0;
}


int main(int argc, char* argv[])
{

	long       thread;  /* Use long in case of a 64-bit system */
	pthread_t* thread_handles; 
	
    	
	
	/* Command line parameter count should be 4 */
	if (argc != 4) { 
		fprintf(stderr, "usage: %s <Size of theArray_ on server> <server ip> <server port>\n", argv[0]);
		exit(0);
	}
	
	int array_size = strtol(argv[1], NULL, 10);
	char *server_ip = argv[2];
	int server_port = strtol(argv[3], NULL, 10);
	
	
	
	pthread_mutex_t mutexes[array_size];
	theArray = (char**)malloc(array_size*sizeof(char*)); 
	
	// memoray allocate array of pointers to strings, and initialize an array of mutexes
	for (int i=0; i<array_size; i++) {
		pthread_mutex_init(&mutexes[i], NULL);
		theArray[i] = (char*)malloc(COM_BUFF_SIZE*sizeof(char)); //str_msg_default;
	}
	
	
	// populate char array
	char str_msg_default[COM_BUFF_SIZE];
	for (int i=0; i<array_size; i++) {
		sprintf(str_msg_default, "String %d: the initial value", i); 
		theArray[i] = str_msg_default; // seems to be working when tested
		//printf(theArray[i]);
	}
	
	
	
	thread_handles = malloc(COM_NUM_REQUEST*sizeof(pthread_t)); // 1000 threads
	
	// create a socket
	int serverFileDescriptor=socket(AF_INET,SOCK_STREAM,0);
	int clientFileDescriptor;
	struct sockaddr_in sock_var_;
	
	/* Initialize socket address and port*/
	sock_var_.sin_addr.s_addr=inet_addr(server_ip);
	sock_var_.sin_port=server_port;
	sock_var_.sin_family=AF_INET;
	
	
	
	if(bind(serverFileDescriptor,(struct sockaddr*)&sock_var_,sizeof(sock_var_))>=0) {
        	printf("socket has been created\n");
        	listen(serverFileDescriptor,2000); 
        	
        	while (1) {
        	
			/* Create threads */
			for (thread = 0; thread < COM_NUM_REQUEST; thread++) {  
				// block at accept() until there is an incoming client connection
				clientFileDescriptor=accept(serverFileDescriptor,NULL,NULL);
				printf("Connected to client %d\n",clientFileDescriptor);
				
				// HERE
				RequestParameters *param_struct = (RequestParameters*) malloc(sizeof(RequestParameters));
				param_struct->clientFileDescriptor = clientFileDescriptor;
				param_struct->mutex_pointer = &mutexes[0];
				// HERE
				
				pthread_create(&thread_handles[thread], NULL, 
					ProcessRequest, (void*) param_struct);  // server opens 1000 threads to handle client requests &mutexes					(void*)(long)clientFileDescriptor
			}
			
			/* Finalize threads */
			for (thread = 0; thread < COM_NUM_REQUEST; thread++) {
			    	pthread_join(thread_handles[thread], NULL); 
			}
        	
        	}
        	close(serverFileDescriptor);
        	
    	} else {
		printf("socket creation failed\n");
	}
	
	free(thread_handles);
	return 0;
}
