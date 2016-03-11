#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <signal.h>

typedef struct range
{
	int start;
	int finish;
}range;

void print_ranges(void*);
void initialize_stacks();
void print_sum(int,int);  

range *ranges;
char *stack;
char *stack_top;
const int STACK_SIZE = 65536;
 
int main(void)
{
	ranges = malloc(2*sizeof(range));
	ranges[0].start = 0;
	ranges[0].finish = 100;
	ranges[1].start = 101;
	ranges[1].finish = 200;
	initialize_stacks();
	int cpid = clone(print_ranges, stack_top, SIGCHLD, ranges);
	waitpid(-1, cpid, NULL);
	int num1 = 2;
	int num2 = 5;
	clone(print_sum, stack_top, SIGCHLD, &num1, &num2);
} 

void print_sum(int num1, int num2)
{
	printf("%d + %d = %d\n", num1, num2. num1+num2);
	_exit(0);
}

void print_ranges(void *args)
{
	range *r = ((range*)args);
	int i;
	for (i = 0; i<2; i++)
	{
		printf("Range at index %d: (%d,%d)\n", i, r[i].start, r[i].finish);  
	}
	_exit(0);
}

void initialize_stacks()
{
	stack = malloc(STACK_SIZE*sizeof(char));
	stack_top = STACK_SIZE+stack;
}
