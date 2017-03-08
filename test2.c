#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

// test2 and test3 are two tests working together
// one is the backup of a guaranteed working version of the other

void Get_dims(int* m_p, int* n_p, int my_rank, MPI_Comm comm);

int main(void) {
	int m, local_m, n, local_n;
	int my_rank, comm_sz;
	MPI_Comm comm;

	MPI_Init(NULL, NULL);
	comm = MPI_COMM_WORLD;
	MPI_Comm_size(comm, &comm_sz);
	MPI_Comm_rank(comm, &my_rank);

	Get_dims(&m, &n, my_rank, comm);

	MPI_Finalize();
	return 0;
}

void Get_dims(
		int*      m_p        /* out */, 
		int*      n_p        /* out */,
		int       my_rank    /* in  */,
		MPI_Comm  comm       /* in  */) {

	if (my_rank == 0) {
		printf("Enter the number of rows\n");
		scanf("%d", m_p);
		printf("Enter the number of columns\n");
		scanf("%d", n_p);
	}
	MPI_Bcast(m_p, 1, MPI_INT, 0, comm);
	MPI_Bcast(n_p, 1, MPI_INT, 0, comm);

}
