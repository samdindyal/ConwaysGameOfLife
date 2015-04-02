/*	"MikMakServer.c"
 *	Assignment 2
 *	CPS590 - Intro to Operating Systems
 *
 *	Written By:		Sam Dindyal
 * 	Description:	Server program for communicating with "MikMak" clients.
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <errno.h>

#include "Mak.h"
#include "message_buffer.h"

//Constants for better legibility
#define GOOD_EXIT	0
#define BAD_EXIT	1

int send_msqid, receive_msqid;							//Send and receive msqids
int arg;									//Dummy variable for pthreads

key_t send_key, receive_key;							//Keys for IPC with clients
pthread_t send_thread, receive_thread, runtime_thread;				//pthreads for IPC with clients

MakList *maks;			//Master list of "Maks"
message_buffer *buffer;		//Message buffer for receiving user input from clients

//Prototypes for all methods
void *send(void*);
void *receive(void*);
void initial_setup();
void *run(void*);

int main(void)
{
	//Initialize all appropriate values and set up message queues
	initial_setup();

	//Create thread for runtime
	if (pthread_create(&runtime_thread, NULL, run, &arg))
	{
		fprintf(stderr, "Error creating thread.\n");
		exit(GOOD_EXIT);
	}
	printf("MikMakServer is now running.\n");

	char temp[16];	//Storage for storing input for safe termination

	//Keep running until ^D is entered
	while(fgets(temp, sizeof(temp), stdin) != NULL){}
	printf("MikMakServer is shutting down...\n");

	//Remove identifiers and shut down "MikMakServer"
	if ((msgctl(receive_msqid, IPC_RMID, NULL) == -1) 
	||  (msgctl(send_msqid, IPC_RMID, NULL) == -1))
	{
		perror("msgctl");
		exit(1);
	}
}

/*	Method Signature:	initial_setup()
 *	Description:		Initialize all appropriate values and set up message queues
 */
void initial_setup()
{
	//Set the keys for queues
	if ((send_key = ftok("MikMakServer.c", 'S')) == -1 
	|| (receive_key = ftok("MikMakServer.c", 'R')) == -1)
	{
		perror("ftok");
		exit(BAD_EXIT);
	}

	//Open queues
	if ((send_msqid = msgget(send_key, 0666 | IPC_CREAT)) == -1
	|| (receive_msqid = msgget(receive_key, 0666 | IPC_CREAT)) == -1)
	{
		perror("msgget");
		exit(BAD_EXIT);
	}

	//Allocate memory for "maks" and "buffer"
	maks = (MakList*)malloc(sizeof(MakList));
	buffer = (message_buffer*)malloc(sizeof(message_buffer));

	//Initialize the size of "maks" to 0
	maks->size = 0;

	//Set the mtypes of "maks" and "buffer" to 1
	maks->mtype = 1;
	buffer->mtype = 1;
}

/*	Method Signature:	send(void*)
 *	Description:		Send the master "MakList" to client(s)
 */
void *send(void *arg)
{
	//Send "maks" to client(s)
	if (msgsnd(send_msqid, maks, sizeof(MakList), 0) == -1)
	{
		perror("msgsnd");
		exit(BAD_EXIT);
	}

	//Terminate method
	return NULL;
}

/*	Method Signature:	receive(void*)
 *	Description:		Receive user input from clients
 */
void *receive(void *arg)
{
	//Receive user input and store it in "buffer"
	if(msgrcv(receive_msqid, buffer, sizeof(message_buffer), 0, 0) == -1)
	{
		perror("msgrcv");
		exit(BAD_EXIT);
	}

	//Terminate method
	return NULL;
}

/*	Method Signature:	run(void*)
 *	Description:		Run "MikMakServer".
 */
void *run(void *arg)
{
	char temp[256];	//Buffer for formatting

	while(1)
	{
		//Create receive pthread
		if (pthread_create(&receive_thread, NULL, receive, &arg))
		{
			fprintf(stderr, "Error creating thread.\n");
			exit(BAD_EXIT);
		}
		
		//Wait for receive pthread to complete
		if(pthread_join(receive_thread, NULL)) 
		{
			fprintf(stderr, "Error joining thread\n");
			exit(BAD_EXIT);
		}

		//Store substring of the user input into "temp" to exclude the dialog input (i.e. 'S', etc.)
		sprintf(temp, "%s", buffer->text+2);
		
		//If the user entered an 'S', make a new "Mak" with "temp" as its text
		if (toupper(buffer->text[0]) == 'S' && maks->size < 10)			
			add_mak(maks, temp);

		//If the user entered a 'U', increment the votes on the user defined "Mak"
		else if (toupper(buffer->text[0]) == 'U')
			maks->votes[atoi(temp)-1]++;

		//If the user entered a 'D'
		else if (toupper(buffer->text[0]) == 'D')
		{
			//Parse index from user input
			int index = atoi(temp)-1;

			//Decrement the votes on the "Mak" at "index"
			maks->votes[index]--;

			//If the votes on a "Mak" have reached -7, or less
			if (maks->votes[index] <= -5)
				//Remove the "Mak" from "maks"
				remove_mak(maks, index);	
		}
 		
 		//If the user issued a refresh
		if (toupper(buffer->text[0]) == 'R')
		{
			//Create send pthread
			if (pthread_create(&send_thread, NULL, send, &arg))
			{
				fprintf(stderr, "Error creating thread.\n");
				exit(BAD_EXIT);
			}
	
			//Wait for the send pthread to complete
			if(pthread_join(send_thread, NULL)) 
			{
				fprintf(stderr, "Error joining thread\n");
				exit(BAD_EXIT);
			}
		}
	}
}


