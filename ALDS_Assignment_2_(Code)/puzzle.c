#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/time.h>

/** 
 * READ THIS DESCRIPTION
 *
 * node data structure: containing state, g, f
 * you can extend it with more information if needed
 */
typedef struct node{
	int state[16];			
	int g;					
	int f;			
} node;

/**
 * Global Variables
 */

// used to track the position of the blank in a state,
// so it doesn't have to be searched every time we check if an operator is applicable
// When we apply an operator, blank_pos is updated
int blank_pos;


// Initial node of the problem
node initial_node;


// Statistics about the number of generated and expendad nodes
unsigned long generated;
unsigned long expanded;


/**
 * The id of the four available actions for moving the blank (empty slot). e.x.
 * Left: moves the blank to the left, etc. 
 */

#define LEFT 0
#define RIGHT 1
#define UP 2
#define DOWN 3
#define RANDOMMOVE 4
#define STATESIZE 16

/*
 * Helper arrays for the applicable function
 * applicability of operators: 0 = left, 1 = right, 2 = up, 3 = down 
 */
int ap_opLeft[]  = { 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1 };
int ap_opRight[]  = { 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0 };
int ap_opUp[]  = { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
int ap_opDown[]  = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 };
int *ap_ops[] = { ap_opLeft, ap_opRight, ap_opUp, ap_opDown };


/* print state */
void print_state( int* s )
{
	int i;
	for( i = 0; i < 16; i++ )
		printf( "%2d%c", s[i], ((i+1) % 4 == 0 ? '\n' : ' ') );
}
      
void printf_comma (long signed int n) {
    if (n < 0) {
        printf ("-");
        printf_comma (-n);
        return;
    }
    if (n < 1000) {
        printf ("%lu", n);
        return;
    }
    printf_comma (n/1000);
    printf (",%03lu", n%1000);
}

/* return the sum of manhattan distances from state to goal */
int manhattan( int* state )
{
	int sum = 0;

	int i = 0;
	for( i = 0; i < STATESIZE; i++)
	{
		if(state[i] != 0)
			sum += (abs((i % 4) - (state[i] % 4)) + abs((i / 4) - (state[i] / 4)));
	}	
	return( sum );
}


/* return 1 if op is applicable in state, otherwise return 0 */
int applicable( int op )
{
       	return( ap_ops[op][blank_pos] );
}


/* apply operator */
void apply( node* n, int op )
{
	int t;

	//find tile that has to be moved given the op and blank_pos
	t = blank_pos + (op == 0 ? -1 : (op == 1 ? 1 : (op == 2 ? -4 : 4)));

	//apply op
	n->state[blank_pos] = n->state[t];
	n->state[t] = 0;
	
	//update blank pos
	blank_pos = t;
}

//Function to compute the reverseAction of the parameter action.
int computeReverseMove(int previousMove)
{
	if(previousMove == 0)
	{
		return 1;
	}
	else if(previousMove == 1)
	{
		return 0;
	}
	else if(previousMove == 2)
	{
		return 3;
	}
	else if(previousMove == 3)
	{
		return 2;
	}
	//4 here stands for the NULL value.
	return 4;
}

/* Recursive IDA */
node* ida( node* node, int threshold, int* newThreshold , int previousMove)
{	
	int i = 0;
	for( i = 0; i <= DOWN; i++)
	{	
		int reverseMove = computeReverseMove(previousMove);
		if(reverseMove != i)
		{
			if(applicable(i) == 1)
			{
				struct node* new_node = (struct node*)malloc(sizeof(struct node));
				generated++;
				struct node* result = (struct node*)malloc(sizeof(struct node));
				int j = 0;
				for(j = 0; j < STATESIZE; j++)
				 	new_node->state[j] = node->state[j];

				/*This variable holds the blank position before any action 
				is applied*/
				int previousBlankPos = blank_pos;

				//Stores the previousMove temporarily
				int temp = previousMove;

				//Storing the action in the previousMove
				previousMove = i;
				apply(new_node, i);
				new_node->g = node->g + 1;
				new_node->f = new_node->g + manhattan(new_node->state);
				if(new_node->f > threshold)
			 	{
			 		if(new_node->f < *newThreshold)
			 			*newThreshold = new_node->f;
			 		free(new_node);
			 	}
				else
				{
					expanded++;
				  	if(manhattan(new_node->state) == 0)
			  			return new_node;
				  	result = ida(new_node, threshold, newThreshold, previousMove);
				  	free(new_node);
					if(result != NULL)
						return result;
				}
			//Resetting the previousMove
			previousMove = temp;

			//Resetting the blank position when backtracking.
			blank_pos = previousBlankPos;
			}
		}	
	}
	return( NULL );
}


/* main IDA control loop */
int IDA_control_loop(  ){
	node* r = NULL;
	
	int threshold;

	//Putting the previous move here for the history, as it is not a pointer, do not need to reinitialize.
	int previousMove = RANDOMMOVE;
	
	/* initialize statistics */
	generated = 0;
	expanded = 0;

	/* compute initial threshold B */
	initial_node.f = threshold = manhattan( initial_node.state );
	printf( "Initial Estimate = %d\nThreshold = ", threshold );
	
	//Our code starts here.
	while(r == NULL)
	{
		int new_threshold = INT_MAX;
		struct node* node = (struct node*)malloc(sizeof(struct node));

		int i = 0;
		for(i = 0; i < STATESIZE; i++)
			node->state[i] = initial_node.state[i];

		node->g = 0;
		r = ida(node, threshold, &new_threshold, previousMove);

		if(r == NULL)
		{
			threshold = new_threshold;
			printf(" %d", threshold);
		}
		free(node);
	}
	if(r)
		return r->g;
	else
		return -1;
}


static inline float compute_current_time()
{
	struct rusage r_usage;
	
	getrusage( RUSAGE_SELF, &r_usage );	
	float diff_time = (float) r_usage.ru_utime.tv_sec;
	diff_time += (float) r_usage.ru_stime.tv_sec;
	diff_time += (float) r_usage.ru_utime.tv_usec / (float)1000000;
	diff_time += (float) r_usage.ru_stime.tv_usec / (float)1000000;
	return diff_time;
}
int main( int argc, char **argv )
{
	int i, solution_length;

	/* check we have a initial state as parameter */
	if( argc != 2 )
	{
		fprintf( stderr, "usage: %s \"<initial-state-file>\"\n", argv[0] );
		return( -1 );
	}


	/* read initial state */
	FILE* initFile = fopen( argv[1], "r" );
	char buffer[256];

	if( fgets(buffer, sizeof(buffer), initFile) != NULL ){
		char* tile = strtok( buffer, " " );
		for( i = 0; tile != NULL; ++i )
			{
				initial_node.state[i] = atoi( tile );
				blank_pos = (initial_node.state[i] == 0 ? i : blank_pos);
				tile = strtok( NULL, " " );
			}		
	}
	else{
		fprintf( stderr, "Filename empty\"\n" );
		return( -2 );

	}
       
	if( i != 16 )
	{
		fprintf( stderr, "invalid initial state\n" );
		return( -1 );
	}

	/* initialize the initial node */
	initial_node.g=0;
	initial_node.f=0;

	print_state( initial_node.state );


	/* solve */
	float t0 = compute_current_time();
	
	solution_length = IDA_control_loop();				

	float tf = compute_current_time();

	/* report results */
	printf( "\nSolution = %d\n", solution_length);
	printf( "Generated = ");
	printf_comma(generated);		
	printf("\nExpanded = ");
	printf_comma(expanded);		
	printf( "\nTime (seconds) = %.2f\nExpanded/Second = ", tf-t0 );
	printf_comma((unsigned long int) expanded/(tf+0.00000001-t0));
	printf("\n\n");

	/* aggregate all executions in a file named report.dat, for marking purposes */
	FILE* report = fopen( "report.dat", "a" );

	fprintf( report, "%s", argv[1] );
	fprintf( report, "\n\tSoulution = %d, Generated = %lu, Expanded = %lu", solution_length, generated, expanded);
	fprintf( report, ", Time = %f, Expanded/Second = %f\n\n", tf-t0, (float)expanded/(tf-t0));
	fclose(report);
	
	return( 0 );
}

