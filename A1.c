/*
 *	Assignment 1
 *	CPS590 - Intro to Operating Systems
 *	Written By:	Sam Dindyal
 *	Description: 	Multithreaded simulation of a grid based on "Conway's Game of Life" using the "clone" call	
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <signal.h>

//Constants for better legibility
#define GOODEXIT	0
#define BADEXIT		1

#define DEAD 		0
#define ALIVE 		1

//Prototypes for all methods
int mod(int, int);
int neighbor_count(int, int);
void print_grid(int**);
void advance_multithread();
void swap_grids();
void initialize_grids();
void initialize_stacks();
void set_ranges();
void print_ranges();
void run();
void advance_single_thread();

int size;	//The size of the grid
int T;		//The number of threads
int *args;	//Arguments entered into the "advance_multithread" method
int *cpids;	//The child process IDs
int **grid1;	//The first grid
int **grid2;	//The second grid
const int STACK_SIZE = 65536;	//The size of the stack

char **stacks;		//An array of stacks for every thread
char **stackTops; 	//An array of the tops of every stack for each thread


typedef struct range	//Struct representing a range with start and finish values
{
	int start;
	int finish;
}range;

range *ranges; 	//The ranges of which each thread covers in the grid in terms of the rows
FILE *file;	//The input file

int main(int argc, char *argv[])
{
	//Check for 1 or 2 arguments 
	if (argc != 3 && argc != 2) 
	{
		//Print to "stderr" and exit with exit code "1"
		fprintf(stderr, "Invalid number of arguments entered! Now exiting.\n");
		exit(BADEXIT); 
	}
	
	//Open the input file and store it in "file"	
	file = fopen(argv[1], "r");
	
	//If file cannot be opened
	if (file == NULL)  
	{
		//Print to "stderr" and exit with exit code "1"
		fprintf(stderr, "Cannot open file: %s\nNow exiting...\n", argv[1]);
		exit(BADEXIT);
	}

	//If two arguments have been entered, use the second for "T"
	if (argc == 3) 
		T = atoi(argv[2]);
	//Use 1 for "T"	
	else 
		T = 1;

	//Read the first value from the input file and store it in "size" 
	fscanf(file, "%d", &size); 

	//Check if value for "size" is in range
        if (size < 3)
	{
		fprintf(stderr, "Grid must be of size 3 or greater. Size entered is:%d\nNow exiting...\n", size);
		exit(BADEXIT);
	}

	//Check if value of "T" is in range
	if (T > size || T < 1)
	{
		fprintf(stderr, "Invalid number of threads! \nThe number of threads must be an integer between 1 and %d inclusive for \"%s\".\nNow exiting...\n", size, argv[1]);
		exit(BADEXIT);
	}
	
	//Initialize "grid1" and "grid2"
	initialize_grids();

	int i, j;
	//Read input file and store its values into "grid1"
	for (i = 0; i<size; i++)
	{
		for(j = 0; j<size; j++)
			fscanf(file, "%d", &grid1[i][j]);	
	}
	//Close the file	
	fclose(file);

	//Set the ranges based on the size of the grid and the amount of threads
	set_ranges();

	//Begin the simulation
	run();
}

//Runs the simulation
void run()
{
	int i, j = 0;	//Incremental values
	//Loop through endlessly until killed
	while(1)
	{
		//If the number of threads is greater than 1 run the simulation with multiple threads
		if (T > 1)
		{
			//Initialize variables for multiple "clone" calls
			cpids = (int*)malloc(T*sizeof(int));
			args = (int*)malloc(T*sizeof(int));
			initialize_stacks();

			//Create "T" threads
			for (i = 0; i<T; i++)
			{
				args[i] = i;
				cpids[i] = clone(advance_multithread, stackTops[i], CLONE_VM|SIGCHLD, (void*)(&args[i]));
			}
			
			//Synchronize all threads
			for (i = 0; i<T; i++)
				 waitpid(-1, cpids[i], NULL);	
		}
		
		//Run the simulation with a single thread
		else		
			advance_single_thread();

		//Swap "grid1" and "grid2"
		swap_grids();
		
		//Print "grid1"		
		print_grid(grid1);
		
		//Print appropriate message regarding the running mode underneath the grid
		if (T > 1)
			printf("Generation %d\n(Multithreaded Mode with %d threads)\n", j+1, T);
		else 
			printf("Generation %d\n(Single Threaded Mode)\n", j+1);

		//Increment "j"
		j++;

		//Sleep for a second
		sleep(1);
	}	
}

//Set the ranges for each thread
void set_ranges()
{
	int x = size/T;		//The width of each, regular, range
	int i;			//Incremental value

	//Allocate memory for an array of "range" structs of "T" size
	ranges = (range*)malloc(T*sizeof(range));

	//Loop through ranges, excluding the last
	for (i = 0; i<=T-1; i++)
	{
		ranges[i].start = i*x;
		ranges[i].finish = x*(i+1);
	}

	//Set last range to fill the rest of available rows
	ranges[T-1].finish = size;
	ranges[T-1].start = ranges[T-2].finish;
}

//Print all ranges of which each thread covers
void print_ranges()
{
	int i;	//Incremental value

	//Loop through and print the index, start and finish of each range
	for (i = 0; i<T; i++)
		printf("The range of index %d is [%d,%d)\n", i, ranges[i].start, ranges[i].finish);
}

//Initialize stacks for multithreading
void initialize_stacks()
{
	int i;		//Incremental value

	//Allocate memory for the arrays "stacks" and "stackTops"
	stacks = (char**)malloc(T*sizeof(char*));
	stackTops = (char**)malloc(T*sizeof(char*));
		
	//Loop through "T" times, initialize all "stacks" and set "stackTops" 
	for (i = 0; i<T; i++)
	{
		stacks[i] = (char*)malloc(STACK_SIZE*sizeof(char));
		stackTops[i] = stacks[i] + STACK_SIZE;
	}
}

//Initialize "grid1" and "grid2"
void initialize_grids()
{
	int i;		//Incremental value

	//Allocate memory for "grid1" and "grid2"
	grid1 = (int**)malloc(size * sizeof(int*));
	grid2 = (int**)malloc(size * sizeof(int*));

	//Allocate memory for each row in "grid1" and "grid2"
	for (i = 0; i<size; i++)
	{
		grid1[i] = (int*)malloc(size * sizeof(int));
		grid2[i] = (int*)malloc(size * sizeof(int));	
	}	
}

//Swap pointers for "grid1" and "grid2"
void swap_grids(void)
{
	int **temp; 	//Temporary 2D array for swap space

	//Backup "grid1" to "temp"		
	temp = grid1;	
	
	//Make "grid1" point to "grid2"	
	grid1 = grid2;	

	//Make "grid2" point to "temp", the backup of the pointer to the original "grid1"
	grid2 = temp;
}

//Print grid with a border while excluding all "0" values and replacing all "1" values with "x"
void print_grid(int **grid)
{
	int i, j; 	//Incremental values
	
	//Clear the screen
	system("clear");

	//Print top side for bounding box based on the size of the grid
	for (i = 0; i<=size*3.115; i++)
		putchar('-');
	putchar('\n');

	//Loop through each row
	for (i = 0; i<size; i++)
	{
		//Start the current row by printing "|" to build the left side of the bounding box
		putchar('|');

		//Loop through the elements of the current row
		for (j = 0; j<size; j++)
		{
			//If the current cell is alive, or the value is "1"
			if (grid[i][j] == 1)
				//Print " x "
				printf(" x ");

			//If the current cell is dead, or the value is "0"
			else
				//Print "   "
				printf("   ");
		}
		//Finish the current row by printing "|" to build the right side of the bounding box
		printf("|\n");
	}

	//Print bottom for bounding box based on the size of the grid
	for (i = 0; i<=size*3.115; i++)
		putchar('-');
	putchar('\n');
}

//Count the alive neighbors of a cell  in "grid1" given its coordinates
int neighbor_count(int x, int y)
{
	//Return sum of surrounding cell values
	return 	grid1[mod(x-1, size)][mod(y-1, size)]
		+grid1[mod(x-1, size)][y]
		+grid1[x][mod(y+1,size)]
		+grid1[mod(x+1, size)][mod(y+1, size)]
		+grid1[mod(x+1, size)][y]
		+grid1[mod(x+1, size)][mod(y-1,size)]
		+grid1[x][mod(y-1,size)]
		+grid1[mod(x-1, size)][mod(y+1, size)];
}

void process_next_generation(int x, int y)
{

	int counter = neighbor_count(x, y); //The surrounding alive cells of the current cell

	//If the current cell is alive
	if(grid1[x][y] == ALIVE)
	{	
		//If the current cell has less than two neighboring cell which are alive
		if (counter < 2)
			//The current cell dies in the next generation
			grid2[x][y] = DEAD;
		
		//If the current cell has two or three neighboring cells which are alive
		else if (counter == 2 || counter == 3)
			//The current cell lives in the next generation
			grid2[x][y] = ALIVE;

		//If the current cell has more than three neighboring cells which are alive
		else if (counter > 3)
			//The current cell dies in the next generation
			grid2[x][y] = DEAD;
	}
	//If the current cell is dead and has exactly three neighboring cells which are alive
	else if (counter == 3)
	//The current cell is reborn in the next generation
		grid2[x][y] = ALIVE;

	else
		//Write over previous values of "grid2" to avoid any interference
		grid2[x][y] = DEAD;
}

//Process the next generation of "grid1" with a single thread and store it in "grid2"
void advance_single_thread()
{
	int i, j; //Incremental values

	//Loop through entire grid
	for (i = 0; i<size; i++)
		for (j = 0; j<size; j++)
			//Process the state of the current cell in "grid1" in the next generation and store it in "grid2"
			process_next_generation(i, j);
}


void advance_multithread(void *arg)
{
	int i, j; 	//Incremental values
	int thread_ID;	//The ID of the current thread

	if (arg != NULL)
		thread_ID = *((int *)arg);
	else
		thread_ID = 0;

	//Loop through rows based on the range of the current thread
	for (i = ranges[thread_ID].start; i<ranges[thread_ID].finish; i++)
	{
		//Loop through current row
		for (j = 0; j<size; j++)
		//Process the state of the current cell in "grid1" in the next generation and store it in "grid2"
			process_next_generation(i, j);
	}

	//Terminate the current thread
	_exit(GOODEXIT);
}

//Calculates the modulus of two numbers in a custom method, like a circular number
int mod(int num1, int num2)
{
	//If "num1" is not negative
	if (num1 >= 0)
		//The mod is calculated normally
		return num1 % num2;
	else
		//If not, the mod is equal to "num2" minus the offset
		return num2+(num1%num2);
}
