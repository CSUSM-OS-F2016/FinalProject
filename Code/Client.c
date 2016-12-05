/*
 Simple udp client
 */
#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <pthread.h>

#define SERVER "127.0.0.1"
#define BUFLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to send data

struct sockaddr_in si_other;
int s, i, slen=sizeof(si_other);
char buf[BUFLEN];
char bufLis[BUFLEN];
char message[BUFLEN];

pthread_mutex_t lock;


void *talking_function(void *arg); //  function for talking thread

void *listen_function(void *arg); //  function for talking thread


void die(char *s)
{
    perror(s);
    exit(1);
}

int main(void)
{
    
    int status;
    pthread_t	tid[2]; // init my one threads
    
    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
    
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
    
    if (inet_aton(SERVER , &si_other.sin_addr) == 0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
    status = pthread_create(&(tid[0]), NULL, talking_function, NULL);
    
    if (status != 0)
    {
        perror("Thread create error");
        exit(EXIT_FAILURE);
    }
    status = pthread_create(&(tid[1]), NULL, listen_function, NULL);
    
    if (status != 0)
    {
        perror("Thread create error");
        exit(EXIT_FAILURE);
    }

    status = pthread_join(tid[1], NULL);
    if (status != 0)
    {
        perror("Thread join error");
        exit(EXIT_FAILURE);
    }
    
    
    close(s);
    return 0;
}
void *talking_function(void *arg)
{
    while(1)
    {
        
        
        pthread_mutex_lock(&lock); // has the lock
        printf("\n Enter message : \n");
        gets(message);
        printf("\n");
        pthread_mutex_unlock(&lock);//released

        //send the message
        if (sendto(s, message, strlen(message) , 0 , (struct sockaddr *) &si_other, slen)==-1)
        {
            die("sendto()");
        }
        
        //receive a reply and print it
        //clear the buffer by filling null, it might have previously received data
        
        //try to receive some data, this is a blocking call
       
        
        
    }
    
    
    pthread_exit(NULL);
}
void *listen_function(void *arg)
{
    while(1)
    {
        
        
        //try to receive some data, this is a blocking call
        if (recvfrom(s, bufLis, BUFLEN, 0, (struct sockaddr *) &si_other, (unsigned int *)&slen) == -1)
        {
            die("recvfrom()");
        }
        
        //print details of the client/peer and the data received
        printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
        printf("Data: %s\n" , bufLis);
        
         printf("Data: %s\n" , bufLis);

        
    }
    
    
    pthread_exit(NULL);
}