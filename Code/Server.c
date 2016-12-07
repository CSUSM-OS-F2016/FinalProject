//The server file
/*
 Simple udp server
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
#include <semaphore.h>

#define BUFLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to listen for incoming data

struct sockaddr_in si_me, si_other;

int s, i, slen = sizeof(si_other) , recv_len;
char buf[BUFLEN];
char bufLis[BUFLEN];
char message[BUFLEN];
char encryptedMessage[BUFLEN];
char decryptedMessage[BUFLEN];
pthread_t	tid[2]; // init thread(s)

int minValue;//base
int maxValue;//cap

void *hearing_function(void *arg); // function for listening thread
void *talking_function(void *arg); // function for listening thread


/*
* HOW TO CALL: provide an plain message variable you would like to encrypt and a *variable to hold the encrypted message
* PURPOSE: Encrypts a plain text message using a caesar cipher algorithm
*/
void encrypt(char encryptedMessage[BUFLEN], char message[BUFLEN]);

/*
* HOW TO CALL: provide your encrypted message variable, and your variable to hold the  *decrypted message
* PURPOSE: decrypts a plain text message using a caesar cipher algorithm reversed
*/
void decrypt(char message[BUFLEN], char encryptedMessage[BUFLEN]);

//To make checks simpler
int verifyVal(char compareVal, int val1, int val2);

//To change the min and max values
void setVals(int min, int max);

//For encryption and decryption checking
void checkBounds(char value);

void die(char *s)
{
    perror(s);
    exit(1);
}

int main(void)
{
    int status;
    //create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }

    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));

    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    //bind socket to port
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }

    status = pthread_create(&(tid[0]), NULL, hearing_function, NULL);

    if (status != 0)
    {
        perror("Thread create error");
        exit(EXIT_FAILURE);
    }
    status = pthread_create(&(tid[1]), NULL, talking_function, NULL);

    if (status != 0)
    {
        perror("Thread create error");
        exit(EXIT_FAILURE);
    }
    status = pthread_join(tid[0], NULL);
    if (status != 0)
    {
        perror("Thread join error");
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
void *hearing_function(void *arg)
{
    while(1)
    {
        fflush(stdout);

        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, (unsigned int *)&slen)) == -1)
        {
            die("recvfrom()");
        }

        //print details of the client/peer and the data received
        printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
        printf("Encrypted Data: %s\n" , buf);
        memset(decryptedMessage, '\0', BUFLEN);
        decrypt(decryptedMessage, buf);//decrypt the message

        printf("decrypted Data: %s\n" , decryptedMessage);
    }


    pthread_exit(NULL);
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
        //now reply the client with the same data
        if (sendto(s, encryptedMessage, sizeof(encryptedMessage), 0, (struct sockaddr*) &si_other, slen) == -1)
        {
            die("sendto()");
        }

         memset(message,'\0', sizeof(message));
    }


    pthread_exit(NULL);
}

/*
* PURPOSE: Encrypts a plain text message using a caesar cipher algorithm
*		  by shifting right through the asci table 4 indexs.
*		  The Series of if statements set the bounds depending on what your ascii value is
* @param encryptedMessage variable and message variable
*/
void encrypt(char encryptedMessage[BUFLEN], char message[BUFLEN]) {
	minValue = 0;//base
	maxValue = 0;//cap
	int key = 0;
	int i = 0;
	int asciIndex = 0;
	while (message[i] != '\0')
  {
		char asciVal = message[i];
		//if statements set boundaries for different cases, whether its a number, or letter etc
	  checkBounds(asciVal);

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
* PURPOSE: decrypts a plain text message using a caesar cipher algorithm reversed
*		  by shifting left through the asci table 4 indexs.
*		  The Series of if statements set the bounds depending on what your ascii value is
* @param encryptedMessage variable and message variable to hold decrypted message variable
*/
void decrypt(char message[BUFLEN], char encryptedMessage[BUFLEN]) {
		//if statements set boundaries for different cases, whether its a number, or letter etc
		minValue = 0;//base
		maxValue = 0;//cap
		int key = 0;
		int i = 0;
		int asciIndex = 0;
		while (encryptedMessage[i] != '\0')
    {
			char asciVal = encryptedMessage[i];
			checkBounds(asciVal);

			if (asciVal == ' ') {  // if asci value is a space assign a space and skip
				message[i] = ' ';
				i++;
				continue;
			}
			asciIndex = asciVal;   //get ascii index for ascii value
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


  /**
  * Complete a standard comparison between two values
  * @param compareVal The variable that we are comparing
  * @param val1       The upper bound (max)
  * @param val2       The lower bound (min)
  */
  int verifyVal(char compareVal, int val1, int val2){
    if(compareVal >= val1 && compareVal <= val2){
      //Set the min to val1 and the max to val2
      setVals(val1, val2);
      return 0;
    }
    return -1;
  }

  /**
  * Sets the min and max values for encryption
  * @param min        The min value to set minValue to
  * @param max        The max value to set maxValue to
  */
  void setVals(int min, int max){
    minValue = min;
    maxValue = max;
  }

  /**
  * Wrapper function for verifyVals
  * @param value      The variable to use as a comparison in verifyVal
  */
  void checkBounds(char value){
  //if statements set boundaries for different cases, whether its a number, or letter etc
    verifyVal(value, 65, 90);      //uppercase
    verifyVal(value, 97, 122);     //lowercase
    verifyVal(value, 48, 57);      //numbers
    verifyVal(value, 33, 47);      //specialChars1
    verifyVal(value, 58, 64);      //specialChars2
    verifyVal(value, 91, 96);      //specialChars3
    verifyVal(value, 123, 126);    //specialChars4
  }
