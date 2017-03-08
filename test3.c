#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

void Read(int* n_p, int my_rank, MPI_Comm comm);
void Print(int n, int my_rank, MPI_Comm comm);

int main(void) {
	int n, local_n;
	int my_rank, comm_sz;
	MPI_Comm comm;

	// char *filename = "xxx";
	// FILE* fp;
	// if (my_rank == 0) {
	// 	printf("my_rank = 0, opening file\n");
	// 	fp = fopen(filename, "r");
	// }	

	MPI_Init(NULL, NULL);
	comm = MPI_COMM_WORLD;
	MPI_Comm_size(comm, &comm_sz);
	MPI_Comm_rank(comm, &my_rank);

	Read(&n, my_rank, comm);
	Print(n, my_rank, comm);

	MPI_Finalize();
	return 0;
}

void Read(
	int* 		n_p 		/* out */,
	int 		my_rank 	/* in  */,
	MPI_Comm 	comm 		/* in  */) {

	if (my_rank == 0) {
		printf("Enter a number\n");
		scanf("%d", n_p);
	}
	MPI_Bcast(n_p, 1, MPI_INT, 0, comm);
}

void Print(
	int 		n 			/* in */,
	int 		my_rank 	/* in */,
	MPI_Comm 	comm 		/* in */) {

	if (my_rank == 0) {
		printf("The number entered was%d\n", n);
	}
}









