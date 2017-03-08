#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

void Check_for_error(int local_ok, char fname[], char message[], 
	MPI_Comm comm);
void Read_input(FILE* fp, int* np, int* local_np, double* errp,
	int my_rank, int comm_sz, MPI_Comm comm);
void Allocate_arrays(double** local_A_pp, double** local_b_pp, 
	double** local_x_pp, int n, int local_n, MPI_Comm comm);
void Read_matrix(FILE* fp, double local_A[], double local_b[],
	double local_x[], int n, int local_n, int my_rank, MPI_Comm comm);
void Print_matrix(double local_A[], double local_b[], 
	double local_x[], int n, int local_n, int my_rank, MPI_Comm comm);

int main(int argc, char *argv[]) {
	
	double* local_A;
	double* local_b;
	double* local_x;
	double err;
	int n, local_n;
	int my_rank, comm_sz;
	MPI_Comm comm;

	if(argc != 2) {
		printf("Incorrect number of arguments\n");
		exit(1);
	}

	char *filename = argv[1];
	FILE* fp;
	if (my_rank == 0)
		fp = fopen(filename, "r");

	MPI_Init(NULL, NULL);
	comm = MPI_COMM_WORLD;
	MPI_Comm_size(comm, &comm_sz);
	MPI_Comm_rank(comm, &my_rank);

	Read_input(fp, &n, &local_n, &err, my_rank, comm_sz, comm);
	Allocate_arrays(&local_A, &local_b, &local_x, n, local_n, comm);
	Read_matrix(fp, local_A, local_b, local_x, n, local_n, my_rank, comm);
	Print_matrix(local_A, local_b, local_x, n, local_n, my_rank, comm);

	free(local_A);
	free(local_b);
	free(local_x);
	if (my_rank == 0)
		fclose(fp);
	MPI_Finalize();
	return 0;
}

void Check_for_error(
	int 		local_ok 	/* in */, 
	char 		fname[] 	/* in */,
	char 		message[] 	/* in */, 
	MPI_Comm 	comm 		/* in */) {
	
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
}

void Read_input(
	FILE* 		fp 			/* in */, 
	int* 		np 			/* out */, 
	int*		local_np 	/* out */,
	double* 	errp 		/* out */,
	int 		my_rank 	/* in */, 
	int 		comm_sz 	/* in */, 
	MPI_Comm 	comm 		/* in */) {

	int local_ok = 1;

	if (my_rank == 0) {
		printf("Enter num\n");
		scanf("%d", np);
		printf("Enter err\n");
		scanf("%d", errp);
	}


	// if (my_rank == 0) {
	// 	fscanf(fp, "%d ", np);
	// 	fscanf(fp, "%lf ", errp);
	// }
		MPI_Bcast(np, 1, MPI_INT, 0, comm);
		MPI_Bcast(errp, 1, MPI_DOUBLE, 0, comm);

	if (*np % comm_sz != 0) local_ok = 0;
	Check_for_error(local_ok, "Read_input", 
		"n must be divisible by comm_sz", comm);
	*local_np = *np / comm_sz;
}

void Allocate_arrays(
	double** 	local_A_pp 	/* out */, 
	double** 	local_x_pp 	/* out */, 
	double** 	local_b_pp 	/* out */, 
	int 		n 			/* in  */,   
	int 		local_n 	/* in  */, 
	MPI_Comm 	comm 		/* in  */) {

	int local_ok = 1;

	*local_A_pp = malloc(local_n*n*sizeof(double));
	*local_x_pp = malloc(local_n*sizeof(double));
	*local_b_pp = malloc(local_n*sizeof(double));

	if (*local_A_pp == NULL || local_x_pp == NULL ||
			local_b_pp == NULL) local_ok = 0;
	Check_for_error(local_ok, "Allocate_arrays",
			"Can't allocate local arrays", comm);
}

void Read_matrix(
	FILE* 		fp 				/* in */,
	double 		local_A[] 		/* out */,
	double 		local_b[] 		/* out */, 
	double 		local_x[] 		/* out */,
	int 		n 				/* in */,
	int 		local_n 		/* in */,
	int 		my_rank 		/* in */, 
	MPI_Comm 	comm 			/* in */) {

	double* A = NULL;
	double* b = NULL;
	double* x = NULL;
	int local_ok = 1;
	int i, j;

	if (my_rank == 0) {
		A = malloc(local_n * n * sizeof(double));
		b = malloc(local_n * sizeof(double));
		if (A == NULL || b == NULL || x == NULL) local_ok = 0;
		Check_for_error(local_ok, "Read_matrix", 
			"Cannot allocate temporary matrix", comm);

		for (i = 0; i < n; i++)
			fscanf(fp, "%lf", &x[i]); // read initial values of x
		for (i = 0; i < n; i++) {
			for (j = 0; j < n; j++)
				fscanf(fp, "%lf", &A[i]); // read coefficients a
			fscanf(fp, "%lf", &b[i]); // read coefficients b
		}
		MPI_Scatter(x, local_n, MPI_DOUBLE, 
			local_x, local_n, MPI_DOUBLE, 0, comm);
		MPI_Scatter(A, local_n * n, MPI_DOUBLE, 
			local_A, local_n, MPI_DOUBLE, 0, comm);
		MPI_Scatter(b, local_n, MPI_DOUBLE,
			local_b, local_n, MPI_DOUBLE, 0, comm);
		free(x);
		free(A);
		free(b);
	} else {
		Check_for_error(local_ok, "Read_matrix", 
			"Cannot allocate temporary matrix", comm);
		MPI_Scatter(x, local_n, MPI_DOUBLE, 
			local_x, local_n, MPI_DOUBLE, 0, comm);
		MPI_Scatter(A, local_n * n, MPI_DOUBLE, 
			local_A, local_n, MPI_DOUBLE, 0, comm);
		MPI_Scatter(b, local_n, MPI_DOUBLE,
			local_b, local_n, MPI_DOUBLE, 0, comm);
	}	
}

void Print_matrix(
	double 		local_A[] 		/* in */,
	double 		local_b[] 		/* in */,
	double 		local_x[] 		/* in */,
	int 		n 				/* in */, 
	int 		local_n 		/* in */,
	int 		my_rank 		/* in */,
	MPI_Comm 	comm 			/* in */) {

	double* A = NULL;
	double* b = NULL;
	double* x = NULL;
	int local_ok = 1;
	int i, j;

	if (my_rank == 0) {
		A = malloc(n * n * sizeof(double));
		b = malloc(n * sizeof(double));
		x = malloc(n * sizeof(double));
		if (A == NULL || b == NULL || x == NULL) local_ok = 0;
		Check_for_error(local_ok, "Print_matrix", 
			"Cannot allocate temporary matrix", comm);
		MPI_Gather(local_A, local_n * n, MPI_DOUBLE,
			A, local_n * n, MPI_DOUBLE, 0, comm);
		MPI_Gather(local_b, local_n, MPI_DOUBLE,
			b, local_n, MPI_DOUBLE, 0, comm);
		MPI_Gather(local_x, local_n, MPI_DOUBLE, 
			x, local_n, MPI_DOUBLE, 0, comm);
		printf("Initial values of Xs are\n");
		for (i = 0; i < n; i++)
			printf("%f\n", x[i]);
		printf("\n\nThe matrix goes:\n");
		for (i = 0; i < n; i++) {
			for (j = 0; j < n; j++) 
				printf("%f", A[i * n + j]);
			printf("%f\n", b[j]);
		}
		printf("%s\n");
		free(A);
		free(b);
	} else {
		Check_for_error(local_ok, "Print_matrix", 
			"Cannot allocate temporary matrix", comm);
		MPI_Gather(local_A, local_n * n, MPI_DOUBLE,
			A, local_n * n, MPI_DOUBLE, 0, comm);
		MPI_Gather(local_b, local_n, MPI_DOUBLE,
			b, local_n, MPI_DOUBLE, 0, comm);		
	}

}

