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

pthread_rwlock_t rwl;
char **theArray;
double times[COM_NUM_REQUEST];
typedef struct {
    int clientFileDescriptor; 
    int requestNumber;
} RequestParameters; 

void *handleRequest(void *args)
{	
    RequestParameters *param_struct = (RequestParameters*) args;
	int clientFileDescriptor = param_struct->clientFileDescriptor;
    char msg[COM_BUFF_SIZE], string_read[COM_BUFF_SIZE];
	double start_time, end_time;
    ClientRequest *rqst;
	rqst = (ClientRequest*) malloc(sizeof(ClientRequest));

    read(clientFileDescriptor,msg,COM_BUFF_SIZE);
	GET_TIME(start_time);
    
	ParseMsg(msg, rqst);
   
	if (!(rqst -> is_read))
    {
      pthread_rwlock_wrlock(&rwl);
      setContent(rqst -> msg, rqst -> pos, theArray);
      pthread_rwlock_unlock(&rwl);  
    }

    pthread_rwlock_rdlock(&rwl);
    getContent(string_read, rqst -> pos, theArray);
    pthread_rwlock_unlock(&rwl);
	
	GET_TIME(end_time);
    write(clientFileDescriptor,string_read,COM_BUFF_SIZE);
    
    close(clientFileDescriptor);
	
	times[param_struct->requestNumber] = start_time - end_time;
	
    return NULL;
}


int main(int argc, char* argv[])
{
    pthread_rwlock_init(&rwl, NULL);
    
    if (argc != 4) {
        fprintf(stderr, "usage: %s <size of array> <server ip> <server port>\n", argv[0]);
        exit(0);
    }

    int arraySize = atoi(argv[1]);
    char* serverIp = argv[2];
    int serverPort = atoi(argv[3]);

    theArray = (char**) malloc(sizeof(char*) * arraySize);
    for(int i = 0; i < arraySize; i++)
    {
        theArray[i] = (char*) malloc(sizeof(char) * COM_BUFF_SIZE);
    }
    for(int i = 0; i < arraySize; i++)
    {
        sprintf(theArray[i], "String %d: the initial value", i);
    }
  
  // for(int i = 0; i < arraySize; i++)
  //   {
  //       printf("%s\n", theArray[i]);
  //   }
  

    struct sockaddr_in sock_var;
    int serverFileDescriptor=socket(AF_INET,SOCK_STREAM,0);
    int clientFileDescriptor;
    int i;
    pthread_t t[COM_NUM_REQUEST];

    sock_var.sin_addr.s_addr=inet_addr(serverIp);
    sock_var.sin_port=serverPort;
    sock_var.sin_family=AF_INET;
    if(bind(serverFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0)
    {
        printf("socket has been created\n");
        listen(serverFileDescriptor,2000); 
        while(1)        //loop infinity
        {
            for(i=0;i<COM_NUM_REQUEST;i++)
			{	
                clientFileDescriptor=accept(serverFileDescriptor,NULL,NULL);
                RequestParameters *param_struct = (RequestParameters*) malloc(sizeof(RequestParameters));
				param_struct->clientFileDescriptor = clientFileDescriptor;
				param_struct->requestNumber = i;

                pthread_create(&t[i],NULL,handleRequest,(void *)param_struct);
            }
			for (i=0;i<COM_NUM_REQUEST;i++)
			{
				pthread_join(t[i], NULL);
			}
			saveTimes(times, COM_NUM_REQUEST);
        }
        close(serverFileDescriptor);
    }
    else{
        printf("socket creation failed\n");
    }
    return 0;
}