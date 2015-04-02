/*	"MikMak.c"
 *	Assignment 2
 *	CPS590 - Intro to Operating Systems
 *
 *	Written By:		Sam Dindyal
 * 	Description:	Client program for receiving "Maks" and sending the user's input from and to the server.	
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
#define	GOOD_EXIT	0
#define BAD_EXIT	1

int send_msqid, receive_msqid; 			//Send and receive msqids
int arg;					//Dummy variable for pthreads

key_t send_key, receive_key;			//Keys for IPC with the server
pthread_t send_thread, receive_thread;		//pthreads for IPC with the server

message_buffer *buffer;		//Message buffer for sending user input to the server
MakList *maks;			//Local list of "Maks"

const char input_dialog[37] = "[R]efresh [U]p [D]own [S]end [E]xit:";	//Input dialog for user interface

//Prototypes for all methods
void initial_setup();
void *send(void*);
void *receive(void*);
void run();
void print_UI();

int main(void)
{
	//Initialize all appropriate values and set up message queues
	initial_setup();

	//Start "MikMak"
	run();
}

/*	Method Signature:	initial_setup()
 *	Description:		Initialize all appropriate values and set up message queues
 */
void initial_setup()
{
	//Set the keys for queues
	if ((send_key = ftok("MikMakServer.c", 'R')) == -1 
	|| (receive_key = ftok("MikMakServer.c", 'S')) == -1)
	{
		perror("ftok");
		exit(BAD_EXIT);
	}

	//Open queues
	if ((send_msqid = msgget(send_key, 0666)) == -1
	|| (receive_msqid = msgget(receive_key, 0666)) == -1)
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
 *	Description:		Send the user input to the server
 */
void *send(void *arg)
{
	int len = strlen(buffer->text);	//Length of the buffer

	//Replace a new line character, if there is one, with a null character
	if (buffer->text[len-1] == '\n')
		buffer->text[len-1] = '\0';	

	//Send the message buffer, containing the user input, to the server
	if (msgsnd(send_msqid, buffer, sizeof(message_buffer), 0) == -1)
	{
		perror("msgsnd");
		exit(BAD_EXIT);
	}

	//Terminate method
	return NULL;
}

/*	Method Signature:	receive(void*)
 *	Description:		Receive an updated "MakList" from the server.
 */
void *receive(void *arg)
{
	//Update the local "MakList", "maks", with the one received from the server
	if(msgrcv(receive_msqid, maks, sizeof(MakList), 0, 0) == -1)
	{
		perror("msgrcv");
		exit(BAD_EXIT);
	}

	//Terminate method
	return NULL;
}

/*	Method Signature:	print_UI()
 *	Description:		Print the user interface.
 */
void print_UI()
{
	//Clear the screen
	system("clear");

	//Print the title
	printf("%-42s %-45s\n--------------------------------------------------------------------------------------------------\n", "", "M I K   M A K");
	int i; //Incremental value

	//Loop through all "Maks" and print them
	for (i = 0; i < maks->size; i++)
		print_mak(maks, i);

	//If there aren't any "Maks" in the list yet
	if (maks->size == 0)
		printf("There are no \"Maks\" yet. Why don't you try adding one?\n");
	
	//Print separator
	printf("--------------------------------------------------------------------------------------------------\n");
	
	//Print the input dialog
	printf("\n%s ", input_dialog);
}

/*	Method Signature:	process_input()
 *	Description:		Processes the user input and communicates with the server accordingly.
 */
void process_input()
{
	//If the user has requested to close "MikMak"
	if (toupper(buffer->text[0]) == 'E')
		exit(GOOD_EXIT);

	//Start pthread for sending user inut
	if (pthread_create(&send_thread, NULL, send, &arg))
	{
		fprintf(stderr, "Error creating thread.\n");
		exit(BAD_EXIT);
	}

	//Wait for send pthread to complete
	if(pthread_join(send_thread, NULL)) {
		fprintf(stderr, "Error joining thread\n");
		exit(BAD_EXIT);
	}
	
	//If the user has requested a refresh
	if (toupper(buffer->text[0]) == 'R')
	{
		//Start pthread for receiving from the server
		if (pthread_create(&receive_thread, NULL, receive, &arg))
		{
			fprintf(stderr, "Error creating thread.\n");
			exit(BAD_EXIT);
		}

		//Wait for the receive pthread to complete
		if(pthread_join(receive_thread, NULL)) {
			fprintf(stderr, "Error joining thread\n");
			exit(BAD_EXIT);
		}

		//Print the UI
		print_UI();
	}
	//If the user hasn't requested a refresh
	else
		//Only print the input dialog
		printf("\n%s ", input_dialog);
}

/*	Method Signature:	run()
 *	Description:		Run "MikMak"
 */
void run()
{
	//Spoof a requested refresh
	buffer->text[0] = 'R';
	process_input();

	//Continue to run until the user terminates "MikMak"
	while(fgets(buffer->text, sizeof(buffer->text), stdin) != NULL)
		process_input();
	
}

