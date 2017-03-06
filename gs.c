#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

/*** Skeleton for Lab 1 ***/

/***** Globals ******/
float **a; /* The coefficients */
float *x;  /* The unknowns */
float *b;  /* The constants */
float err; /* The absolute relative error */
int num = 0;  /* number of unknowns */


/****** Function declarations */
void check_matrix(); /* Check whether the matrix will converge */
void get_input();  /* Read input from file */

/********************************/



/* Function definitions: functions are ordered alphabetically ****/
/*****************************************************************/

/*-------------------------------------------------------------------
 * Function:  Check_for_error
 * Purpose:   Check whether any process has found an error.  If so,
 *            print message and terminate all processes.  Otherwise,
 *            continue execution.
 * In args:   local_ok:  1 if calling process has found an error, 0
 *               otherwise
 *            fname:     name of function calling Check_for_error
 *            message:   message to print if there's an error
 *            comm:      communicator containing processes calling
 *                       Check_for_error:  should be MPI_COMM_WORLD.
 *
 * Note:
 *    The communicator containing the processes calling Check_for_error
 *    should be MPI_COMM_WORLD.
 */
void Check_for_error(
		int       local_ok   /* in */, 
		char      fname[]    /* in */,
		char      message[]  /* in */, 
		MPI_Comm  comm       /* in */) {

	int ok;

	MPI_Allreduce(&local_ok, &ok, 1, MPI_INT, MPI_MIN, comm);
	if (ok == 0) {
		int my_rank;
		MPI_Comm_rank(comm, &my_rank);
		if (my_rank == 0) {
			fprintf(stderr, "Proc %d > In %s, %s\n", my_rank, fname, 
					message);
			fflush(stderr);
		}
		MPI_Finalize();
		exit(-1);
	}
}  /* Check_for_error */




/* 
	 Conditions for convergence (diagonal dominance):
	 1. diagonal element >= sum of all other elements of the row
	 2. At least one diagonal element > sum of all other elements of the row
 */
void check_matrix()
{
	int bigger = 0; /* Set to 1 if at least one diag element > sum  */
	int i, j;
	float sum = 0;
	float aii = 0;
	
	for(i = 0; i < num; i++)
	{
		sum = 0;
		aii = fabs(a[i][i]);
		
		for(j = 0; j < num; j++)
			if(j != i)
				sum += fabs(a[i][j]);
			 
		if(aii < sum)
		{
			printf("The matrix will not converge.\n");
			exit(1);
		}
		
		if(aii > sum)
			bigger++;
		
	}
	
	if(!bigger )
	{
		 printf("The matrix will not converge\n");
		 exit(1);
	}
}


/******************************************************/
/* Read input from file */
/* After this function returns:
 * a[][] will be filled with coefficients and you can access them using a[i][j] for element (i,j)
 * x[] will contain the initial values of x
 * b[] will contain the constants (i.e. the right-hand-side of the equations
 * num will have number of variables
 * err will have the absolute error that you need to reach
 */
void get_input(char filename[])
{
	FILE * fp;
	int i,j;  
 
	fp = fopen(filename, "r");
	if(!fp)
	{
		printf("Cannot open file %s\n", filename);
		exit(1);
	}

	fscanf(fp,"%d ",&num);
	fscanf(fp,"%f ",&err);

	/* Now, time to allocate the matrices and vectors */
	a = (float**)malloc(num * sizeof(float*));
	if(!a)
	{
		printf("Cannot allocate a!\n");
		exit(1);
	}

	for(i = 0; i < num; i++) 
	{
		a[i] = (float *)malloc(num * sizeof(float)); 
		if(!a[i])
		{
		printf("Cannot allocate a[%d]!\n",i);
		exit(1);
		}
	}
 
	x = (float *) malloc(num * sizeof(float));
	if(!x)
	{
		printf("Cannot allocate x!\n");
		exit(1);
	}


	b = (float *) malloc(num * sizeof(float));
	if(!b)
	{
		printf("Cannot allocate b!\n");
		exit(1);
	}

	/* Now .. Filling the blanks */ 

	/* The initial values of Xs */
	for(i = 0; i < num; i++)
		fscanf(fp,"%f ", &x[i]);

	for(i = 0; i < num; i++)
	{
		for(j = 0; j < num; j++)
			fscanf(fp,"%f ",&a[i][j]);
	 
		/* reading the b element */
		fscanf(fp,"%f ",&b[i]);
	}

	fclose(fp); 

}


/************************************************************/


int main(int argc, char *argv[])
{

	int i;
	int nit = 0; /* number of iterations */
	int my_rank, comm_sz;
	MPI_Comm comm;

	MPI_Init(NULL, NULL);
	comm = MPI_COMM_WORLD;
	MPI_Comm_size(comm, &comm_sz);
	MPI_Comm_rank(comm, &my_rank);


	if(argc != 2)
	{
		printf("Usage: gsref filename\n");
		exit(1);
	}

	/* Read the input file and fill the global data structure above */ 
	get_input(argv[1]);

	/* Check for convergence condition */
	/* This function will exit the program if the coffeicient will never converge to 
	* the needed absolute error. 
	* This is not expected to happen for this programming assignment.
	*/
	check_matrix();	

	/* Writing to the stdout */
	/* Keep that same format */
	for(i = 0; i < num; i++)
		printf("%f\n",x[i]);

	printf("total number of iterations: %d\n", nit);

	exit(0);

}
