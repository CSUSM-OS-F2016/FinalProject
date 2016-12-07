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
char encryptedMessage[BUFLEN];
char decryptedMessage[BUFLEN];
pthread_mutex_t lock;


void *talking_function(void *arg); //  function for talking thread

void *listen_function(void *arg); //  function for talking thread


/*HOW TO CALL: provide an plain message variable you would like to encrypt and a *variable to hold the encrypted message
*PURPOSE: Encrypts a plain text message using a caesar cipher algorithm
*/
void encrypt(char encryptedMessage[BUFLEN], char message[BUFLEN]);

/*HOW TO CALL: provide your encrypted message variable, and your variable to hold the  *decrypted message
*PURPOSE: decrypts a plain text message using a caesar cipher algorithm reversed
*/
void decrypt(char message[BUFLEN], char encryptedMessage[BUFLEN]);

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



        printf("\n Enter message : \n");
        gets(message);
        printf("\n");
        printf("Before Encryption: %s \n", message);
        memset(encryptedMessage, '\0', BUFLEN);
        encrypt(encryptedMessage, message);//encrypt message
        printf("Encrypted Message: %s \n", encryptedMessage);
        //send the message
        if (sendto(s, encryptedMessage, strlen(encryptedMessage) , 0 , (struct sockaddr *) &si_other, slen)==-1)
        {
            die("sendto()");
        }

        //receive a reply and print it
        //clear the buffer by filling null, it might have previously received data

        //try to receive some data, this is a blocking call
        memset(message,'\0', sizeof(message));


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
        printf("Received packet from %s:%d\n ", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
        printf("Encrypted Message: %s\n" , bufLis);
        memset(decryptedMessage, '\0', BUFLEN);
        decrypt(decryptedMessage, bufLis);//decrypt the message
        printf("Decrypted Message: %s\n" , decryptedMessage);
        //printf("Data: %s  \n" , bufLis);
        memset(bufLis,'0',BUFLEN);



    }


    pthread_exit(NULL);
}

/*
*PURPOSE: Encrypts a plain text message using a caesar cipher algorithm
*		  by shifting right through the asci table 4 indexs.
*		  The Series of if statements set the bounds depending on what your ascii value is
*@param encryptedMessage variable and message variable
*/
void encrypt(char encryptedMessage[BUFLEN], char message[BUFLEN]) {
	int minValue;//base
	int maxValue;//cap
	int key = 0;
	int i = 0;
	int asciIndex = 0;
	while (message[i] != '\0') {
		char asciVal = message[i];
		//if statements set boundaries for different cases, whether its a number, or letter etc
		if (asciVal >= 65 && asciVal <= 90) {//uppercase
			minValue = 65;
			maxValue = 90;
		}
		if (asciVal >= 97 && asciVal <= 122) {//lowercase
			minValue = 97;
			maxValue = 122;
		}
		if (asciVal >= 48 && asciVal <= 57) {//numbers
			minValue = 48;
			maxValue = 57;
		}
		if (asciVal >= 33 && asciVal <= 47) {//specialChars1
			minValue = 33;
			maxValue = 47;
		}
		if (asciVal >= 58 && asciVal <= 64) {//specialChars2
			minValue = 58;
			maxValue = 64;
		}
		if (asciVal >= 91 && asciVal <= 96) {//specialChars3
			minValue = 91;
			maxValue = 96;
		}
		if (asciVal >= 123 && asciVal <= 126) {//specialChars4
			minValue = 123;
			maxValue = 126;
		}
		if (asciVal == ' ') {// if asci value is a space, assign it space and skip to next iteration
			encryptedMessage[i] = ' ';
			i++;
			continue;
		}
		asciIndex = asciVal;// convert asciVal to the asciii Index
		asciIndex = (asciIndex + 4);// Shifts ascii index 4 times value
		if (asciIndex > maxValue) {//if your index exceeds max value, round robin around the ascii table
			asciIndex = asciIndex % (maxValue);//round-robin
			asciIndex = asciIndex+(minValue-1);// add to your base
		}
		asciVal = asciIndex;//makesure its not below the min value
		encryptedMessage[i] = asciVal;
		i++;
	}
}
  /*
*PURPOSE: decrypts a plain text message using a caesar cipher algorithm reversed
*		  by shifting left through the asci table 4 indexs.
*		  The Series of if statements set the bounds depending on what your ascii value is
*@param encryptedMessage variable and message variable to hold decrypted message variable
*/
void decrypt(char message[BUFLEN], char encryptedMessage[BUFLEN]) {
		//if statements set boundaries for different cases, whether its a number, or letter etc
		int minValue;//base
		int maxValue;//cap
		int key = 0;
		int i = 0;
		int asciIndex = 0;
		while (encryptedMessage[i] != '\0') {
			char asciVal = encryptedMessage[i];

			if (asciVal >= 65 && asciVal <= 90) {//uppercase
				minValue = 65;
				maxValue = 90;
			}
			if (asciVal >= 97 && asciVal <= 122) {//lowercase
				minValue = 97;
				maxValue = 122;
			}
			if (asciVal >= 48 && asciVal <= 57) {//numbers
				minValue = 48;
				maxValue = 57;
			}
			if (asciVal >= 33 && asciVal <= 47) {//specialChars1
				minValue = 33;
				maxValue = 47;
			}
			if (asciVal >= 58 && asciVal <= 64) {//specialChars2
				minValue = 58;
				maxValue = 64;
			}
			if (asciVal >= 91 && asciVal <= 96) {//specialChars3
				minValue = 91;
				maxValue = 96;
			}
			if (asciVal >= 123 && asciVal <= 126) {//specialChars4
				minValue = 123;
				maxValue = 126;
			}

			if (asciVal == ' ') {// if asci value is a space assign a space and skip
				message[i] = ' ';
				i++;
				continue;
			}
			asciIndex = asciVal;//get ascii index for ascii value
			asciIndex = (asciIndex - 4);// asci index shifs left 4 times

			if (asciIndex < minValue) {//if your index goes below min
				asciIndex = (minValue - asciIndex); //Find out how far below min it is
				asciIndex= (maxValue+1) - asciIndex;// subtract that from max +1
			}
			asciVal = asciIndex;//get the ascii value for your index
			message[i] = asciVal;
			i++;
    }
		}
