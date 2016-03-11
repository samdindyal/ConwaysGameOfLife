#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <signal.h>

#define BADEXIT 	1
#define GOODEXIT 	0
#define ALIVE 		1
#define DEAD 		0

int num_threads;
int size;
int **current_gen;
int **next_gen;
int **ranges;
int *cpids;
char **stack;
char **stack_top;
const int STACK_SIZE = 65536;

FILE *input_file;

void initialize_grids();
void initialize_stacks();
void print_grid(int**);
void set_ranges();
void print_ranges();
void swap_grids();
void calculate_next_gen(void*);
int count_neighbours(int,int);
int circular_number(int,int);



int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "Invalid number of arguments entered.\nNow exiting...\n");
		exit(BADEXIT);
	}		

	num_threads = atoi(argv[2]);
	input_file = fopen(argv[1], "r");

	if (input_file == NULL)
	{
		fprintf(stderr, "File \"%s\" cannot be read!\nNow exiting...\n", argv[1]);
		exit(BADEXIT);
	}

	fscanf(input_file, "%d", &size);

	if (num_threads > size || num_threads < 1)
	{
		fprintf(stderr, "Invalid thread number entry!\nNow exiting...\n");
		exit(BADEXIT);
	}

	initialize_grids();

	int i, j;
	for (i = 0; i<size; i++)
		for (j = 0; j<size; j++)
			fscanf(input_file, "%d", &current_gen[i][j]);

	print_grid(current_gen);
	set_ranges();
	initialize_stacks();

	cpids = (int*)malloc(num_threads*sizeof(int));

	while(1)
	{
		for (i = 0; i < num_threads; i++)
			cpids[i] = clone(calculate_next_gen, stack_top[i], CLONE_VM | SIGCHLD, &ranges[i]);
		for (i = 0; i < num_threads; i++)
			waitpid(-1, cpids[i], NULL);
		swap_grids();
		print_grid(current_gen);
		sleep(1);
	}




	exit(GOODEXIT);


}

void initialize_grids()
{
	int i, j;
	current_gen = (int**)malloc(size*sizeof(int*));
	next_gen = (int**)malloc(size*sizeof(int*));
	for (i = 0; i<size; i++)
	{
		current_gen[i] = (int*)malloc(size*sizeof(int));
		next_gen[i] = (int*)malloc(size*sizeof(int));

	}
}

void print_grid(int **grid)
{
	int i, j;
	printf("--------------------\n");
	for (i = 0; i<size; i++)
	{
		putchar('|');
		for (j = 0; j<size; j++)
		{
			if (grid[i][j] == 1)
				putchar('x');
			else
				putchar(' ');
		}
		putchar('|');
		putchar('\n');
	}	
	printf("--------------------\n");
}

void set_ranges()
{
	int i, j;
	int range_width = size/num_threads;

	ranges = (int**)malloc(num_threads*sizeof(int*));
	for (i = 0; i<num_threads; i++)
	{
		ranges[i] = (int*)malloc(2*sizeof(int));
		if (num_threads == 1)	//If there is only one thread, first range covers entire array
		{
			ranges[i][0] = 0;
			ranges[i][1] = size;
		}
		else if (i < num_threads-1)
		{
			ranges[i][0] = i*range_width;		//Starting point of the range i (inclusive)
			ranges[i][1] = (i+1)*range_width;	//Finishing point of range (not inclusive)

		}
		else
		{
			ranges[i][0] = ranges[i-1][1];
			ranges[i][1] = size;
		}

	}
}

void print_ranges()
{
	int i;
	for (i = 0; i<num_threads; i++)
		printf("The range at index %i = [%d,%d)\n", i, ranges[i][0], ranges[i][1]);
}

void swap_grids()
{
	int **temp;
	temp = current_gen;
	current_gen = next_gen;
	next_gen = temp;
}

void calculate_next_gen(void *arg)
{
	int *current_range = (int*)arg;

	int i, j;

	for (i = current_range[0]; i < current_range[1]; i++)
	{
		for (j = 0; j < size; j++)
		{
			int neighbour_count = count_neighbours(i, j);
			if (current_gen[i][j] == ALIVE)
			{
				if (neighbour_count < 2 || neighbour_count > 3)
					next_gen[i][j] = DEAD;
				else
					next_gen[i][j] = ALIVE;
			}
			else if (neighbour_count == 3)
				next_gen[i][j] = ALIVE;
			else
				next_gen[i][j] = DEAD;
		}
	}
	_exit(GOODEXIT);
}

int count_neighbours(int x, int y)
{
	return current_gen[circular_number(x-1, size)][circular_number(y-1, size)]
			+ current_gen[circular_number(x-1, size)][y]
			+ current_gen[x][circular_number(y+1, size)]
			+ current_gen[circular_number(x+1, size)][circular_number(y+1, size)]
			+ current_gen[circular_number(x+1, size)][y]
			+ current_gen[circular_number(x+1, size)][y-1]
			+ current_gen[x][circular_number(y-1, size)]
			+ current_gen[circular_number(x-1, size)][circular_number(y+1, size)];
}

int circular_number(int num1, int num2)
{
	if (num1 >= 0 && num2 >= 0)
		return num1%num2;
	else
		return num2 + (num1%num2);
}

void initialize_stacks()
{
	stack = (char**)malloc(num_threads*sizeof(char*));
	stack_top = (char**)malloc(num_threads*sizeof(char*));

	int i;
	for (i = 0; i < num_threads; i++)
	{
		stack[i] = (char*)malloc(STACK_SIZE*sizeof(char));
		stack_top = STACK_SIZE + stack[i];
	}
}

