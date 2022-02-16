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


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
char **theArray;
int NUM_STR_;

void *ImplementRequest(void *args)
{
    int clientFileDescriptor=(int)args;
    char msg[COM_BUFF_SIZE];
    char string_read[COM_BUFF_SIZE];
    ClientRequest *rqst;
	  rqst = (ClientRequest*) malloc(sizeof(ClientRequest));

    read(clientFileDescriptor,msg,COM_BUFF_SIZE);
    //printf(request)
    ParseMsg(msg, rqst);

	if (rqst -> is_read == 0)
    {
      pthread_mutex_lock(&mutex);
      setContent(rqst -> msg, rqst -> pos, theArray);
      char string_read[COM_BUFF_SIZE];
      getContent(string_read, rqst -> pos, theArray);
      write(clientFileDescriptor,string_read,COM_BUFF_SIZE);
      pthread_mutex_unlock(&mutex);


    }
  else{
    pthread_mutex_lock(&mutex);
    char string_read[COM_BUFF_SIZE];
    getContent(string_read, rqst -> pos, theArray);

    write(clientFileDescriptor,string_read,COM_BUFF_SIZE);
    pthread_mutex_unlock(&mutex);

  }


    close(clientFileDescriptor);
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
    pthread_mutex_lock(&mutex);
    theArray = (char**) malloc(sizeof(char*) *NUM_STR_);

    for(int i = 0; i <NUM_STR_; i++)
    {

        theArray[i] = (char*) malloc(sizeof(char) * COM_BUFF_SIZE);

    }
    pthread_mutex_unlock(&mutex);
    for(int i = 0; i < NUM_STR_; i++)
    {
        sprintf(theArray[i], "String %d: the initial value", i);
    }


    thread_handles = malloc(COM_NUM_REQUEST*sizeof(pthread_t));
    /* Initialize socket address and port*/
    sock_var.sin_addr.s_addr=inet_addr(server_ip);
    sock_var.sin_port=server_port;
    sock_var.sin_family=AF_INET;

    /* Create threads */
    for (thread = 0; thread < COM_NUM_REQUEST; thread++)
                //GET_TIME(startTime[i]);
    //             clientFileDescriptor=accept(serverFileDescriptor,NULL,NULL);
    //             //printf("Connected to client %d\n",clientFileDescriptor);
            pthread_create(&thread_handles[thread], NULL,
                ImplementRequest, (void*) thread);

    /* Finalize threads */
    for (thread = 0; thread < COM_NUM_REQUEST; thread++)
            pthread_join(thread_handles[thread], NULL);
            //GET_TIME(endTime[i]);
            //     times[i] = startTime[i] - endTime[i];

    //  saveTimes(times, COM_NUM_REQUEST);
            //     }
            //     close(serverFileDescriptor);
            // }

    free(thread_handles);
    return 0;




} // end main
