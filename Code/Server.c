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

#define MSGLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to listen for incoming data

struct sockaddr_in si_me, si_other;

int s, i, slen = sizeof(si_other) , recv_len;
char buf[MSGLEN];
char recievedMessage[MSGLEN];
char message[MSGLEN];
char encryptedMessage[MSGLEN];
char decryptedMessage[MSGLEN];
pthread_t	tid[2]; // init thread(s)

int minValue;//base
int maxValue;//cap

void *hearing_function(void *arg); // function for listening thread
void *talking_function(void *arg); // function for listening thread


/*
* HOW TO CALL: provide an plain message variable you would like to encrypt and a *variable to hold the encrypted message
* PURPOSE: Encrypts a plain text message using a caesar cipher algorithm
*/
void encrypt(char encryptedMessage[MSGLEN], char message[MSGLEN]);

/*
* HOW TO CALL: provide your encrypted message variable, and your variable to hold the  *decrypted message
* PURPOSE: decrypts a plain text message using a caesar cipher algorithm reversed
*/
void decrypt(char message[MSGLEN], char encryptedMessage[MSGLEN]);

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
  //create a UDP socket
      /**
      * This 'if' statement creates a new socket using a default constructor
      *       • Constructor Parameters:
      *         • AF_INET:      Address Family that is used to designate that our
      *                         socket can communicate with. There are 8 different
      *                         familes including: PF_UNIX, PF_SYSTEM, and PF_INET6
      *         • SOCK_DGRAM:   The specified type of the socket. This parameter specifies
      *                         the semantics of communication. There are 3 types.
      *         • IPPROTO_UDP:  Finally, we are referencing the protocol we are using.
      *                         In this case, we are using IP Protocol: UDP.
      *       • Constructor returns the descriptor of the socket. If the value is -1,
      *                         there was an error in its creation and we must kill the
      *                         program safely.
      */
  if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
      die("socket");

  // zero out the structure
  memset((char *) &si_me, 0, sizeof(si_me));

  si_me.sin_family = AF_INET;                 // Set the socket family explicitly.
  si_me.sin_port = htons(PORT);               // Convert the values between host and network
                                              // byte order, using the specified port above.
  si_me.sin_addr.s_addr = htonl(INADDR_ANY);  // set the address to any incoming connection

  //bind socket to port
  if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
      die("bind");

      //  Create the thread and assign it to the hearing_function
      //  If there is an error, kill the program
  if (pthread_create(&(tid[0]), NULL, hearing_function, NULL) != 0)
      die("Thread create error");

      //  Create a thread and assign it to the talking_function
      //  If there is an error, kill the program
  if (pthread_create(&(tid[1]), NULL, talking_function, NULL) != 0)
      die("Thread create error");

      //  Sync the threads
  if (pthread_join(tid[0], NULL) != 0)
      die("Thread join error");

      //  Sync the threads
  if (pthread_join(tid[1], NULL) != 0)
      die("Thread join error");

    close(s);
    return 0;
}
void *hearing_function(void *arg)
{
    while(1)
    {
        fflush(stdout);

        //try to receive some data, this is a blocking call
        /**
        * This 'if' statement recieves the message from the predescribed socket so it can be decrypted
        *     • @param s          Current socket
        *     • @param recievedMessage     Restricted buffer
        *     • @param MSGLEN     The size of the buffer length
        *     • @param 0          Any flags required
        *     • @param &si_other  The socket address of the server
        *     • @param &slen      The length of the address
        *  @return The number of bytes recieved. -1 if an error occured
        */
        if ((recv_len = recvfrom(s, recievedMessage, MSGLEN, 0, (struct sockaddr *) &si_other, (unsigned int *)&slen)) == -1)
            die("recvfrom()");

        //print details of the client/peer and the data received
        //printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
        //printf("Encrypted Data: %s\n" , buf);
        memset(decryptedMessage, '\0', MSGLEN);                 // Allocate and set memory for the decrupted message
        decrypt(decryptedMessage, recievedMessage);                         // decrypt the message

        printf("\033[0;34m Client: %s \033[0m \n" , decryptedMessage);              // Print the data
    }


    pthread_exit(NULL);                                         // Exit the thread
}

/**
* Function used to send a message
*/
void *talking_function(void *arg)
{
    while(1)
    {
        printf("\n \033[0;33m  Enter message : \033[0m \n");
        gets(message);
        printf("\n");

        //printf("Before Encryption: %s \n", message);
        memset(encryptedMessage, '\0', MSGLEN);
        encrypt(encryptedMessage, message);//encrypt message
        //printf("Encrypted Message: %s \n", encryptedMessage);
        //now reply the client with the same data
             /**
             * This 'if' statement is used to send the message
             *     • @param s                  The current Socket
             *     • @param encruptedMessage   The encryptedMessage to send
             *     • @param strlen             The size of the message
             *     • @param 0                  Any flags for the message
             *     • @param &si_other          Socket address
             *     • @param slen               The length of the sent message
             * Returns -1 if fails.
             */
        if (sendto(s, encryptedMessage, sizeof(encryptedMessage), 0, (struct sockaddr*) &si_other, slen) == -1)
            die("sendto()");

         memset(message,'\0', sizeof(message));
    }


    pthread_exit(NULL);                 // Exit the thread
}

/*
* PURPOSE: Encrypts a plain text message using a caesar cipher algorithm
*		  by shifting right through the asci table 4 indexs.
*		  The Series of if statements set the bounds depending on what your ascii value is
* @param encryptedMessage variable and message variable
*/
void encrypt(char encryptedMessage[MSGLEN], char message[MSGLEN]) {
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
		asciIndex = asciVal;                    // convert asciVal to the asciii Index
		asciIndex = (asciIndex + 4);            // Shifts ascii index 4 times value
		if (asciIndex > maxValue) {             // if your index exceeds max value, round robin around the ascii table
			asciIndex = asciIndex % (maxValue);   // round-robin
			asciIndex = asciIndex+(minValue-1);   // add to your base
		}
		asciVal = asciIndex;                    //makesure its not below the min value
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
void decrypt(char message[MSGLEN], char encryptedMessage[MSGLEN]) {
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
			asciIndex = asciVal;                     // get ascii index for ascii value
			asciIndex = (asciIndex - 4);             // asci index shifs left 4 times

			if (asciIndex < minValue) {              // if your index goes below min
				asciIndex = (minValue - asciIndex);    // Find out how far below min it is
				asciIndex= (maxValue+1) - asciIndex;   // subtract that from max +1
			}
			asciVal = asciIndex;                     // get the ascii value for your index
			message[i] = asciVal;
			i++;
    }
	}


  /**
  * Complete a standard comparison between two values
  * @param compareVal The variable that we are comparing
  * @param val1       The upper bound (max)
  * @param val2       The lower bound (min)
  * @return           If the change was made
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
