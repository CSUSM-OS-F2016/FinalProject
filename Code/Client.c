//The client file
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


int minValue;//base
int maxValue;//cap


void *talking_function(void *arg); //  function for talking thread

void *listen_function(void *arg); //  function for talking thread


/*
* HOW TO CALL: provide an plain message variable you would like to encrypt and a
*   variable to hold the encrypted message
* PURPOSE: Encrypts a plain text message using a caesar cipher algorithm
*/
void encrypt(char encryptedMessage[BUFLEN], char message[BUFLEN]);

/*
* HOW TO CALL: provide your encrypted message variable, and your variable to
*    hold the  *decrypted message
* PURPOSE: decrypts a plain text message using a caesar cipher algorithm reversed
*/
void decrypt(char message[BUFLEN], char encryptedMessage[BUFLEN]);

//To make checks simpler
int verifyVal(char compareVal, int val1, int val2);

//To change the min and max values
void setVals(int min, int max);

//For encryption and decryption checking
void checkBounds(char value);

/**
* Error handling method (default). shows the error and stops exceution
* @param s       String to be displayed as the error
*/
void die(char *s)
{
    perror(s);
    exit(1);
}

/**
* Main invocation of program
*     • Creates the sockets
*     • Creates the sending and recieving threads
*/
int main(void)
{
    int status;
    pthread_t	tid[2]; // init my threads

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
    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        die("socket");

    /** Create memory with the byte value of the socket */
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;                      // Set the socket family explicitly.
    si_other.sin_port = htons(PORT);                    // Convert the values between host and network
                                                        // byte order, using the specified port above.

    /**
    * This 'if' statement converts the presentation format address to network format
    *     • It returns 1 if the address was valid for the specified address family
    *     • It returns 0 if the address was not parseable, or
    *     • It returns -1 if some system error occured (which errno is set).
    * This statement is valid for AF_INET and AF_INET6
    */
    if (inet_aton(SERVER , &si_other.sin_addr) == 0)
        die("inet_aton() failed");

    //  Create a thread and assign it to the talking_function
    //  If there is an error, kill the program
    if ((status = pthread_create(&(tid[0]), NULL, talking_function, NULL)) != 0)
        die("Thread create error");

    //  Create the thread and assign it to the listen_function
    //  If there is an error, kill the program
    if ((status = pthread_create(&(tid[1]), NULL, listen_function, NULL)) != 0)
        die("Thread create error");

    //  Sync the threads
    if ((pthread_join(tid[1], NULL)) != 0)
        die("Thread join error");

    //  Close the socket
    close(s);
    return 0;
}

/**
* Function used to send a message
*/
void *talking_function(void *arg)
{
  //Always listen for input
    while(1)
    {
        printf("\n\033[0;34m Enter message : \033[0m \n");                      // Prompt the user for input
        gets(message);                                        // Get the message from the stream
        printf("\n\033[0;34m Before Encryption: %s \033[0m \n", message);        // Show original message
        memset(encryptedMessage, '\0', BUFLEN);               // Set the bytes for the message
        encrypt(encryptedMessage, message);                   // Encrypt the message
        printf("\033[0;34m Encrypted Message: %s  \033[0m \n", encryptedMessage); // Print the encrypted message

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
        if (sendto(s, encryptedMessage, strlen(encryptedMessage) , 0 , (struct sockaddr *) &si_other, slen)==-1)
            die("sendto()");

        //receive a reply and print it
        //clear the buffer by filling null, it might have previously received data

        //try to receive some data, this is a blocking call
        memset(message,'\0', sizeof(message));
    }

    /** Exit the current thread */
    pthread_exit(NULL);
}

/**
* Function to always listen for text from the server
*/
void *listen_function(void *arg)
{
    // Always listen
    while(1)
    {
        /** try to receive some data, this is a blocking call */
        /**
        * This 'if' statement recieves the message from the predescribed socket so it can be decrypted
        *     • @param s          Current socket
        *     • @param bufLis     Restricted buffer
        *     • @param BUFLEN     The size of the buffer length
        *     • @param 0          Any flags required
        *     • @param &si_other  The socket address of the server
        *     • @param &slen      The length of the address
        *  @return The number of bytes recieved. -1 if an error occured
        */
        if (recvfrom(s, bufLis, BUFLEN, 0, (struct sockaddr *) &si_other, (unsigned int *)&slen) == -1)
            die("recvfrom()");

        //print details of the client/peer and the data received
        //printf("Received packet from %s:%d\n ", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
        //printf("Encrypted Message: %s\n" , bufLis);
        memset(decryptedMessage, '\0', BUFLEN);                 // Allocate and set memory for the decrupted message
        decrypt(decryptedMessage, bufLis);                      // decrypt the message

        printf("\033[0;33m Server: %s \033[0m \n" , decryptedMessage);   // Display the decrypted Message
        memset(bufLis,'0',BUFLEN);                              // Set memory for the registered buffer
    }

    pthread_exit(NULL);                                         //Exit the thread
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
		char value = message[i];
	  checkBounds(value);

		if (value == ' ') {
      // if asci value is a space, assign it space and skip to next iteration
			encryptedMessage[i] = ' ';
			i++;
			continue;
		}
		asciIndex = value;                      // convert value to the asciii Index
		asciIndex = (asciIndex + 4);            // Shifts ascii index 4 times value
		if (asciIndex > maxValue)
    {
      //if your index exceeds max value, round robin around the ascii table
			asciIndex = asciIndex % (maxValue);   // round-robin
			asciIndex = asciIndex+(minValue-1);   // add to your base
		}
		value = asciIndex;                      //makesure its not below the min value
		encryptedMessage[i] = value;
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
    minValue = 0;                             //base
    maxValue = 0;                            //cap
		int key = 0;
		int i = 0;
		int asciIndex = 0;
		while (encryptedMessage[i] != '\0') {
			char value = encryptedMessage[i];

			checkBounds(value);

			if (value == ' ') {                    // if asci value is a space assign a space and skip
				message[i] = ' ';
				i++;
				continue;
			}
			asciIndex = value;                     // get ascii index for ascii value
			asciIndex = (asciIndex - 4);           // asci index shifs left 4 times

			if (asciIndex < minValue) {            // if your index goes below min
				asciIndex = (minValue - asciIndex);  // Find out how far below min it is
				asciIndex= (maxValue+1) - asciIndex; // subtract that from max +1
			}
			value = asciIndex;                     //get the ascii value for your index
			message[i] = value;
			i++;
    }
	}


  /**
  * Complete a standard comparison between two values
  * @param compareVal The variable that we are comparing
  * @param val1       The upper bound (max)
  * @param val2       The lower bound (min)
  * @return           If the change was made or not
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
