#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

void Read(FILE* fp, int* n_p, double* err_p, int my_rank, MPI_Comm comm);
void Print(int n, double err, int my_rank, MPI_Comm comm);

int main(int argc, char* argv[]) {
	int n;
	double err;
	int my_rank, comm_sz;
	MPI_Comm comm;

	char *filename = argv[1];
	FILE* fp;
	if (my_rank == 0) {
		printf("my_rank = 0, opening file\n");
		fp = fopen(filename, "r");
	}

	MPI_Init(NULL, NULL);
	comm = MPI_COMM_WORLD;
	MPI_Comm_size(comm, &comm_sz);
	MPI_Comm_rank(comm, &my_rank);

	Read(fp, &n, &err, my_rank, comm);
	Print(n, err, my_rank, comm);

	MPI_Finalize();
	return 0;
}

void Read(
	FILE* 		fp 			/* in */,
	int* 		n_p 		/* out */,
	double* 	err_p 		/* out */,
	int 		my_rank 	/* in  */,
	MPI_Comm 	comm 		/* in  */) {

	if (my_rank == 0) {
		printf("Reading numbers...\n");
		fscanf(fp, "%d", n_p);
		fscanf(fp, "%lf", err_p);
	}
	MPI_Bcast(n_p, 1, MPI_INT, 0, comm);
	MPI_Bcast(err_p, 1, MPI_DOUBLE, 0, comm);
}

void Print(
	int 		n 			/* in */,
	double 		err 		/* in */,
	int 		my_rank 	/* in */,
	MPI_Comm 	comm 		/* in */) {

	if (my_rank == 0) {
		printf("The number entered was %d\n", n);
		printf("Value of error was %lf\n", err);
	}
}









