#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

// this version works - read input and print output

void Read_top(FILE* fp, int* n_p, int* local_n_p, double* err_p,
	int my_rank, int comm_sz, MPI_Comm comm);
void Print_top(int n, double err, int my_rank, MPI_Comm comm);
void Allocate_arrays(double** local_A, double** local_b, double** local_x,
	int n, int local_n, MPI_Comm comm);
void Read_content(FILE* fp, double local_A[], double local_b[], 
	double local_x[], int n, int local_n, int my_rank, MPI_Comm comm);
void Print_content(double local_A[], double local_b[], double local_x[],
	int n, int local_n, int my_rank, MPI_Comm comm);

int main(int argc, char* argv[]) {
	double* local_A;
	double* local_b;
	double* local_x;
	int n, local_n;
	double err;
	int my_rank, comm_sz;
	MPI_Comm comm;

	char *filename = argv[1];

	FILE* fp;
	if (my_rank == 0) {
		// printf("my_rank = 0, opening file\n");
		fp = fopen(filename, "r");
	}

	MPI_Init(NULL, NULL);
	comm = MPI_COMM_WORLD;
	MPI_Comm_size(comm, &comm_sz);
	MPI_Comm_rank(comm, &my_rank);

	Read_top(fp, &n, &local_n, &err, my_rank, comm_sz, comm);
	Allocate_arrays(&local_A, &local_b, &local_x, n, local_n, comm);
	Read_content(fp, local_A, local_b, local_x, n, local_n, my_rank, comm);
	Print_top(n, err, my_rank, comm);
	Print_content(local_A, local_b, local_x, n, local_n, my_rank, comm);
	if (my_rank == 0)
		fclose(fp);
	MPI_Finalize();
	return 0;
}

void Read_top(
	FILE* 		fp 			/* in  */,
	int* 		n_p 		/* out */,
	int* 		local_n_p 	/* out */,
	double* 	err_p 		/* out */,
	int 		my_rank 	/* in  */,
	int 		comm_sz 	/* in  */,
	MPI_Comm 	comm 		/* in  */) {

	if (my_rank == 0) {
		printf("Reading numbers...\n");
		fscanf(fp, "%d", n_p);
		fscanf(fp, "%lf", err_p);
	}
	MPI_Bcast(n_p, 1, MPI_INT, 0, comm);
	MPI_Bcast(err_p, 1, MPI_DOUBLE, 0, comm);
	*local_n_p = *n_p/comm_sz;
}

void Print_top(
	int 		n 			/* in  */,
	double 		err 		/* in  */,
	int 		my_rank 	/* in  */,
	MPI_Comm 	comm 		/* in  */) {

	if (my_rank == 0) {
		printf("The number entered was %d\n", n);
		printf("Value of error was %lf\n", err);
	}
}

void Allocate_arrays(
	double** 	local_A_pp 	/* out */,
	double** 	local_b_pp 	/* out */,
	double** 	local_x_pp 	/* out */,
	int 		n 			/* in  */,
	int 		local_n		/* in  */,
	MPI_Comm 	comm 		/* in  */) {

	*local_A_pp = malloc(local_n*n*sizeof(double));
	*local_b_pp = malloc(local_n*sizeof(double));
	*local_x_pp = malloc(local_n*sizeof(double));
}
void Read_content(
	FILE* 		fp 			/* in  */,
	double 		local_A[] 	/* out */,
	double 		local_b[] 	/* out */,
	double 		local_x[] 	/* out */,
	int 		n 			/* in  */,
	int 		local_n 	/* in  */,
	int 		my_rank 	/* in  */,
	MPI_Comm 	comm 		/* in  */) {

	double* A = NULL;
	double* b = NULL;
	double* x = NULL;
	int i, j;

	if (my_rank == 0) {
		A = malloc(n*n*sizeof(double));
		b = malloc(n*sizeof(double));
		x = malloc(n*sizeof(double));
		printf("Logging in values of x\n");
		for (i = 0; i < n; i++)
			fscanf(fp, "%lf", &x[i]);
		for (i = 0; i < n; i++) {
			for (j = 0; j < n; j++)
				fscanf(fp, "%lf", &A[i*n+j]);
			fscanf(fp, "%lf", &b[i]);
		}
		MPI_Scatter(A, n*local_n, MPI_DOUBLE, 
			local_A, n*local_n, MPI_DOUBLE, 0, comm);
		MPI_Scatter(b, local_n, MPI_DOUBLE, 
			local_b, local_n, MPI_DOUBLE, 0, comm);
		MPI_Scatter(x, local_n, MPI_DOUBLE, 
			local_x, local_n, MPI_DOUBLE, 0, comm);
		free(A);
		free(b);
		free(x);
	} else {
		MPI_Scatter(A, n*local_n, MPI_DOUBLE, 
			local_A, n*local_n, MPI_DOUBLE, 0, comm);
		MPI_Scatter(b, local_n, MPI_DOUBLE, 
			local_b, local_n, MPI_DOUBLE, 0, comm);
		MPI_Scatter(x, local_n, MPI_DOUBLE, 
			local_x, local_n, MPI_DOUBLE, 0, comm);
	}
}

void Print_content(
	double 		local_A[] 	/* in */,
	double 		local_b[] 	/* in */,
	double 		local_x[] 	/* in */,	
	int 		n 			/* in */,
	int 		local_n 	/* in */, 
	int 		my_rank 	/* in */,
	MPI_Comm 	comm 		/* in */) {

	double* A = NULL;
	double* b = NULL;
	double* x = NULL;
	int i, j;

	if (my_rank == 0) {
		A = malloc(n*n*sizeof(double));
		b = malloc(n*sizeof(double));
		x = malloc(n*sizeof(double));
		MPI_Gather(local_A, n*local_n, MPI_DOUBLE,
			A, n*local_n, MPI_DOUBLE, 0, comm);
		MPI_Gather(local_b, local_n, MPI_DOUBLE,
			b, local_n, MPI_DOUBLE, 0, comm);
		MPI_Gather(local_x, local_n, MPI_DOUBLE,
			x, local_n, MPI_DOUBLE, 0, comm);
		
		printf("\nPrinting values of x\n");
		for (i = 0; i < n; i++)
			printf("%f ", x[i]);
		printf("\nPrinting content of matrix\n");
		for (i = 0; i < n; i++) {
			for (j = 0; j < n; j++)
				printf("%f ", A[i*n+j]);
			printf("%f ", b[i]);
		}
		free(A);
		free(b);
		free(x);
	} else {
		MPI_Gather(local_A, n*local_n, MPI_DOUBLE,
			A, n*local_n, MPI_DOUBLE, 0, comm);
		MPI_Gather(local_b, local_n, MPI_DOUBLE,
			b, local_n, MPI_DOUBLE, 0, comm);
		MPI_Gather(local_x, local_n, MPI_DOUBLE,
			x, local_n, MPI_DOUBLE, 0, comm);
	}
}




