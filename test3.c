#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

//

void Check_for_error(int local_ok, char fname[], char message[],
	MPI_Comm comm);
void Read_top(FILE* fp, int* n_p, int* local_n_p, double* err_p,
	int my_rank, int comm_sz, MPI_Comm comm);
void Print_top(int n, double err, int my_rank, MPI_Comm comm);
void Allocate_arrays(double** local_A, double** local_b, double** local_x,
	double** local_y, int n, int local_n, MPI_Comm comm);
void Read_content(FILE* fp, double local_A[], double local_b[], 
	double local_x[], int n, int local_n, int my_rank, MPI_Comm comm);
void Print_content(double local_A[], double local_b[], double local_x[],
	int n, int local_n, int my_rank, MPI_Comm comm);
void Update_x(double local_x[], double local_y[], double local_A[], 
	double local_b[], double err, int n, int local_n, int my_rank, 
	int comm_sz, MPI_Comm comm);

int main(int argc, char* argv[]) {
	double* local_A;
	double* local_b;
	double* local_x;
	double* local_y; // buffer storage for local_x during odd phases
	int n, local_n;
	double err;
	int my_rank, comm_sz;
	MPI_Comm comm;

	char *filename = argv[1];

	FILE* fp;
	if (my_rank == 0) {
		fp = fopen(filename, "r");
	}

	MPI_Init(NULL, NULL);
	comm = MPI_COMM_WORLD;
	MPI_Comm_size(comm, &comm_sz);
	MPI_Comm_rank(comm, &my_rank);

	Read_top(fp, &n, &local_n, &err, my_rank, comm_sz, comm);
	Allocate_arrays(&local_A, &local_b, &local_x, &local_y, n, local_n, comm);
	Read_content(fp, local_A, local_b, local_x, n, local_n, my_rank, comm);
	// Print_top(n, err, my_rank, comm);
	// Print_content(local_A, local_b, local_x, n, local_n, my_rank, comm);
	Update_x(local_x, local_y, local_A, local_b, err, n, local_n,
		my_rank, comm_sz, comm);
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
			fprintf(stderr, "Proc %d Func %s, %s\n", my_rank, fname, message);
			fflush(stderr);
		}
		MPI_Finalize();
		exit(-1);
	}
}

void Read_top(
	FILE* 		fp 			/* in  */,
	int* 		n_p 		/* out */,
	int* 		local_n_p 	/* out */,
	double* 	err_p 		/* out */,
	int 		my_rank 	/* in  */,
	int 		comm_sz 	/* in  */,
	MPI_Comm 	comm 		/* in  */) {

	int local_ok = 1;

	if (my_rank == 0) {
		// printf("Reading numbers...\n");
		fscanf(fp, "%d", n_p);
		fscanf(fp, "%lf", err_p);

	}
	MPI_Bcast(n_p, 1, MPI_INT, 0, comm);
	MPI_Bcast(err_p, 1, MPI_DOUBLE, 0, comm);
	if (*n_p % comm_sz != 0) local_ok = 0;
	Check_for_error(local_ok, "Read_top", 
		"number of equations must be divisible by comm_sz", comm);
	*local_n_p = *n_p/comm_sz;
}

void Print_top(
	int 		n 			/* in  */,
	double 		err 		/* in  */,
	int 		my_rank 	/* in  */,
	MPI_Comm 	comm 		/* in  */) {

	if (my_rank == 0) {
		printf("Number of equations: %d\n", n);
		printf("Allowance for error: %lf\n", err);
	}
}

void Allocate_arrays(
	double** 	local_A_pp 	/* out */,
	double** 	local_b_pp 	/* out */,
	double** 	local_x_pp 	/* out */,
	double** 	local_y_pp	/* out */,
	int 		n 			/* in  */,
	int 		local_n		/* in  */,
	MPI_Comm 	comm 		/* in  */) {

	// printf("Allocating arrays...\n");
	int local_ok = 1;

	*local_A_pp = malloc(local_n*n*sizeof(double));
	*local_b_pp = malloc(local_n*sizeof(double));
	*local_x_pp = malloc(local_n*sizeof(double));
	*local_y_pp = malloc(local_n*sizeof(double));

	if (*local_A_pp == NULL || *local_b_pp == NULL ||
		*local_x_pp == NULL || *local_y_pp == NULL) local_ok = 0;
	Check_for_error(local_ok, "Allocate_arrays",
		"Cannot allocate local arrays", comm);
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

	// printf("Reading content...\n");
	double* A = NULL;
	double* b = NULL;
	double* x = NULL;
	int i, j;
	int local_ok = 1;

	if (my_rank == 0) {
		A = malloc(n*n*sizeof(double));
		b = malloc(n*sizeof(double));
		x = malloc(n*sizeof(double));
		if (A == NULL || b == NULL || x == NULL) local_ok = 0;
		Check_for_error(local_ok, "Read_content",
			"Cannot allocate temporary arrays", comm);
		
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
		Check_for_error(local_ok, "Read_content",
			"Cannot allocate temporary arrays", comm);		
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

	// printf("Printing content...\n");

	double* A = NULL;
	double* b = NULL;
	double* x = NULL;
	int i, j;
	int local_ok = 1;

	if (my_rank == 0) {
		A = malloc(n*n*sizeof(double));
		b = malloc(n*sizeof(double));
		x = malloc(n*sizeof(double));
		if (A == NULL || b == NULL || x == NULL) local_ok = 0;
		Check_for_error(local_ok, "Print_content",
			"Cannot allocate temporary arrays", comm);		
		MPI_Gather(local_A, n*local_n, MPI_DOUBLE,
			A, n*local_n, MPI_DOUBLE, 0, comm);
		MPI_Gather(local_b, local_n, MPI_DOUBLE,
			b, local_n, MPI_DOUBLE, 0, comm);
		MPI_Gather(local_x, local_n, MPI_DOUBLE,
			x, local_n, MPI_DOUBLE, 0, comm);
		
		printf("\nPrinting values of x\n");
		for (i = 0; i < n; i++)
			printf("%f ", x[i]);
		printf("\n\nPrinting content of matrix\n");
		for (i = 0; i < n; i++) {
			for (j = 0; j < n; j++)
				printf("%f ", A[i*n+j]);
			printf("\t\t%f\n", b[i]);
		}
		free(A);
		free(b);
		free(x);
	} else {
		Check_for_error(local_ok, "Print_content",
			"Cannot allocate temporary arrays", comm);		
		MPI_Gather(local_A, n*local_n, MPI_DOUBLE,
			A, n*local_n, MPI_DOUBLE, 0, comm);
		MPI_Gather(local_b, local_n, MPI_DOUBLE,
			b, local_n, MPI_DOUBLE, 0, comm);
		MPI_Gather(local_x, local_n, MPI_DOUBLE,
			x, local_n, MPI_DOUBLE, 0, comm);
	}
}

void Update_x(
	double 		local_x[] 	/* in  */,
	double 		local_y[] 	/* out */,
	double 		local_A[] 	/* in  */,
	double 		local_b[] 	/* in  */,
	double 		err 		/* in  */,
	int 		n 			/* in  */,
	int 		local_n 	/* in  */, 
	int 		my_rank 	/* in  */,
	int 		comm_sz 	/* in  */,
	MPI_Comm 	comm 		/* in  */) {

	int phase = 0;
	int i, j;

	double* x = NULL;
	int* test = NULL;
	int* local_test = NULL;
	x = malloc(n*sizeof(double));
	test = malloc(n*sizeof(int));
	local_test = malloc(n*sizeof(int));

	while (1) {
		if (phase > 10 || phase < 0) {
			printf("Process %d breaking at phase = 5\n", my_rank);
			break;
		}	
		if (phase % 2 == 0) {
			phase++;
			MPI_Allgather(local_x, local_n, MPI_DOUBLE, 
				x, local_n, MPI_DOUBLE, comm);

			// if (my_rank == 0) {
			// 	printf("At phase = %d proc = %d, we have x containing:\n", phase, my_rank);
			// 	for (i = 0; i < n; i++)
			// 		printf("%f\n", x[i]);
			// 	printf("\n");
			// }	

			for (i = 0; i < local_n; i++) {
				local_y[i] = 0;
				double tsum = 0;
				int pos = my_rank*local_n+i;
				for (j = 0; j < n; j++) {
					if (pos != j) // local_y[i] is not x[j]
						tsum += (x[j]*local_A[i*n+j]);
				}
				tsum = local_b[i] - tsum;
				local_y[i] = tsum / local_A[i*n+pos];
				if (fabs((local_y[i] - local_x[i]) / local_y[i]) <= err)
					local_test[i] = 0;
				else
					local_test[i] = 1;

				// printf("--after: At phase %d proc %d, pos is %d, local_xy[%d] is %f\n", phase, my_rank, pos, i, local_y[i]);
			} // values updated
			// printf("At phase = %d proc = %d, we have test containing:\n", phase, my_rank);
			// for (i = 0; i < local_n; i++)
			// 	printf("%d\n", local_test[i]);
			// printf("\n");
			MPI_Barrier(comm); // wait for all processes to complete update
		}
		else {
			phase++;
			MPI_Allgather(local_y, local_n, MPI_DOUBLE, 
				x, local_n, MPI_DOUBLE, comm);

			// if (my_rank == 0) {
			// 	printf("At phase = %d proc = %d, we have x containing:\n", phase, my_rank);
			// 	for (i = 0; i < n; i++)
			// 		printf("%f\n", x[i]);
			// 	printf("\n");
			// }

			for (i = 0; i < local_n; i++) {
				local_x[i] = 0;
				double tsum = 0;
				int pos = my_rank*local_n+i;
				for (j = 0; j < n; j++) {
					if (pos != j) // local_x[i] is not x[j]
						tsum += (x[j]*local_A[i*n+j]);
				}
				tsum = local_b[i] - tsum;
				local_x[i] = tsum / local_A[i*n+pos];
				if (fabs((local_x[i] - local_y[i]) / local_x[i]) <= err)
					local_test[i] = 0;
				else
					local_test[i] = 1;

				// printf("--after: At phase %d proc %d, local_xy[%d] is %f\n", phase, my_rank, i, local_y[i]);
			} // values updated
			// printf("At phase = %d proc = %d, we have test containing:\n", phase, my_rank);
			// for (i = 0; i < local_n; i++)
			// 	printf("%d\n", local_test[i]);
			// printf("\n");
			MPI_Barrier(comm); // wait for all processes to complete update
		}

		MPI_Allgather(local_test, local_n, MPI_INT,
			test, local_n, MPI_INT, comm);
		int cnt = 0;
		for (i = 0; i < n; i++)
			cnt += test[i];
		if (cnt == 0)
			// printf("***** cnt == 0 \n");
		if (cnt == 0 && my_rank == 0) {
			// printf("Enteres cnt = 0, my_rank = 0\n");
			if (phase % 2 == 0) {
				MPI_Allgather(local_x, local_n, MPI_DOUBLE,
					x, local_n, MPI_DOUBLE, comm);
			} else {
				MPI_Allgather(local_y, local_n, MPI_DOUBLE,
					x, local_n, MPI_DOUBLE, comm);
			}
			for (j = 0; j < n; j++)
				printf("%f\n", x[j]);
			printf("total number of iterations: %d\n", phase);
			phase = -10;
		}
		MPI_Barrier(comm);
		MPI_Bcast(phase, 1, MPI_INT, 0, comm);
	}
}
















