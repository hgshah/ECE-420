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


pthread_mutex_t mutex;
double times[COM_NUM_REQUEST];
char **theArray;
int NUM_STR_;
double times[COM_NUM_REQUEST];
typedef struct {
    int clientFileDescriptor;
    int requestNumber;
} RequestParameters;


void *ImplementRequest(void *args)
{

    int clientFileDescriptor=(int)args;
    char msg[COM_BUFF_SIZE];
    char string_read[COM_BUFF_SIZE];
    double start_time, end_time;
    ClientRequest *rqst;
	  rqst = (ClientRequest*) malloc(sizeof(ClientRequest));

    read(clientFileDescriptor,msg,COM_BUFF_SIZE);
    GET_TIME(start_time);
    //printf(request)
    ParseMsg(msg, rqst);

	if (rqst -> is_read == 0)
    {
      pthread_mutex_lock(&mutex);
      setContent(rqst -> msg, rqst -> pos, theArray);
      pthread_mutex_unlock(&mutex);


    }

    pthread_mutex_lock(&mutex);
    getContent(string_read, rqst -> pos, theArray);
    pthread_mutex_unlock(&mutex);
    GET_TIME(end_time);
    write(clientFileDescriptor,string_read,COM_BUFF_SIZE);





    close(clientFileDescriptor);
    times[clientFileDescriptor] = start_time - end_time;
    return NULL;
}
int main(int argc, char* argv[]){

    struct sockaddr_in sock_var;
    long       thread;  /* Use long in case of a 64-bit system */
    pthread_t* thread_handles;

    /* Command line parameter count should be 4 */
    if (argc != 4){
        fprintf(stderr, "usage: %s <Size of theArray_ on server> <server ip> <server port>\n", argv[0]);
        exit(0);
    }
    NUM_STR_ = strtol(argv[1], NULL, 10);
    char *server_ip = argv[2];
    int server_port = strtol(argv[3], NULL, 10);

    theArray = (char**) malloc(sizeof(char*) *NUM_STR_);
    pthread_mutex_init(&mutex, NULL);
    for(int i = 0; i <NUM_STR_; i++)
    {

        theArray[i] = (char*) malloc(sizeof(char) * COM_BUFF_SIZE);

    }

    for(int i = 0; i < NUM_STR_; i++)
    {
        sprintf(theArray[i], "String %d: the initial value", i);
    }


    thread_handles = malloc(COM_NUM_REQUEST*sizeof(pthread_t));
    /* Initialize socket address and port*/
    sock_var.sin_addr.s_addr=inet_addr(server_ip);
    sock_var.sin_port=server_port;
    sock_var.sin_family=AF_INET;
    int serverFileDescriptor=socket(AF_INET,SOCK_STREAM,0);
     int clientFileDescriptor;
     int i;
    if(bind(serverFileDescriptor,(struct sockaddr*)&sock_var,sizeof(sock_var))>=0)
    {
        printf("socket has been created\n");
        listen(serverFileDescriptor,2000);
        while(1)        //loop infinity
        {
          for (thread = 0; thread < COM_NUM_REQUEST; thread++){

            clientFileDescriptor=accept(serverFileDescriptor,NULL,NULL);
          //  printf("Connected to client %d\n",clientFileDescriptor);

            pthread_create(&thread_handles[thread], NULL,
            ImplementRequest, (void *)(long)clientFileDescriptor);
          }

          for (thread = 0; thread < COM_NUM_REQUEST; thread++){
            pthread_join(thread_handles[thread], NULL);

          }
          saveTimes(times, COM_NUM_REQUEST);

        }
        /* Finalize threads */


        close(serverFileDescriptor);
    }else{
        printf("socket creation failed\n");
    }

    free(thread_handles);
    return 0;




} // end main
