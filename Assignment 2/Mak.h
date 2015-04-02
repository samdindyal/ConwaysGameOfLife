/*	"Mak.h"
 *	Assignment 2
 *	CPS590 - Intro to Operating Systems
 *
 *	Written By:		Sam Dindyal
 * 	Description:	Header file for a list of "Maks" and functions corresponding to it.
 */ 

#include <string.h>

//"MakList" definition
typedef struct{
	long mtype;			//mtype for message queue
	char maks[10][256];		//Storage for 10 "Maks"
	int votes[10];			//Votes of "Maks" in the list (Indices correspond to that of the "maks")
	int size;			//Size of the list
}MakList;

/*	Method Signature: 	add_mak(MakList*, char*)
 *	Description: 		Add a "Mak" to a list of "Maks" containing the passed string.
 */
void add_mak(MakList *list, char *mak)
{
	//Copy the text of the new "Mak" into the end of the list
	strcpy(list->maks[list->size],mak);

	//Set the votes to a default value (0)
	list->votes[list->size] = 0;

	//Increment the size
	list->size++;
}

/*	Method Signature:	remove_mak(MakList*, int)
 *	Description:		Remove a "Mak" of "index" from the list.
 */
void remove_mak(MakList *list, int index)
{
	char temp[256]; 	//Empty variable for clearing strings

	//Copy an empty string into the list at "index"
	strcpy(list->maks[index], temp);

	//If index isn't pointing to the last element
	if(index+1 != list->size)	
	{
		int i;	//Incremental value

		//Loop through the list from "index" to the end
		for (i = index; i < list->size-1; i++)
		{
			//Shift each element to its preceding index
			strcpy(list->maks[i], list->maks[i+1]);
			list->votes[i] = list->votes[i+1];

			//Reset the old index
			strcpy(list->maks[i+1], temp);
			list->votes[i+1] = 0;
		}	
	}

	//Decrement the size
	list->size--;
}


/*	Method Signature:	print_mak(MakList*, int)
 *	Description:		Print a mak at a given index with the appropriate formatting.
 */
void print_mak(MakList *list, int index)
{
	char str1[5];	//String buffer

	//Copy the index into "str1"
	sprintf(str1, "%d", index+1);

	//Add a period to the end of "str1"
	strcat(str1, ".");
	char str2[5]; //String buffer
	
	//Add "+" accordingly to the "str2"
	sprintf(str2, "%s%d", list->votes[index] > 0 ? "+" : "", list->votes[index]);

	//Print the "mak", along with its votes and index number, with the appropriate formatting
	printf("%-5s %-5s %s\n", str1, str2, list->maks[index]);
}
