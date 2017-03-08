#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

// // void read(FILE* fp, int* np, int my_rank, int comm_sz, MPI_Comm comm);
// void read(int* np, int my_rank, int comm_sz, MPI_Comm comm);
// void print(int n, int my_rank, MPI_Comm comm);


// // void read_input(FILE* fp, int* np, int* local_np, double* errp,
// 	// int my_rank, int comm_sz, MPI_Comm comm);
// // void allocate_arrays(double** local_A_pp, double** local_b_pp, 
// // 	double** local_x_pp, int n, int local_n, MPI_Comm comm);
// // void read_matrix(FILE* fp, double local_A[], double local_b[],
// // 	double local_x[], int n, int local_n, int my_rank, MPI_Comm comm);
// // void print_matrix(double local_A[], double local_b[], 
// // 	double local_x[], int n, int local_n, int my_rank, MPI_Comm comm);

// int main(int argc, char *argv[]) {
	
// 	int n, local_n;
// 	int my_rank, comm_sz;
// 	MPI_Comm comm;

// 	MPI_Init(NULL, NULL);
// 	comm = MPI_COMM_WORLD;
// 	MPI_Comm_size(comm, &comm_sz);
// 	MPI_Comm_rank(comm, &my_rank);


// 	// if(argc != 2) {
// 	// 	printf("Incorrect number of arguments\n");
// 	// 	exit(1);
// 	// }

// 	// char *filename = argv[1];

// 	// FILE* fp;
// 	// if (my_rank == 0) {
// 	// 	printf("my_rank = 0, opening file\n");
// 	// 	fp = fopen(filename, "r");
// 	// }

// 	comm_sz = 1;
// 	// read(fp, &n, my_rank, comm_sz, comm);
// 	read(&n, my_rank, comm_sz, comm);
// 	print(n, my_rank, comm);

// 	// if (my_rank == 0) {
// 	// 	printf("my_rank = 0, closing file\n");
// 	// 	fclose(fp);
// 	// }
// 	MPI_Finalize();

// 	return 0;
// }

// void read(
// 	// FILE* 		fp 			/* in */, 
// 	int* 		np 			/* out */,
// 	int  		my_rank 	/* in */, 
// 	int 		comm_sz 	/* in */,
// 	MPI_Comm 	comm 		/* in */) {

// 	if (my_rank == 0) {
// 		printf("Enter num: \n");
// 		scanf("%d", np);
		
// 	}
// 	MPI_Bcast(np, 1, MPI_INT, 0, comm);
// }


// void print(
// 	int 		n 			/* in */,
// 	int 		my_rank 	/* in */, 
// 	MPI_Comm 	comm 		/* in */) {

// 	if (my_rank == 0) {
// 		printf("printing num: %d\n", n);
// 	}
// }




void Get_dims(int* m_p, int* local_m_p, int* n_p, int* local_n_p,
		int my_rank, int comm_sz, MPI_Comm comm);

int main(void) {
	int m, local_m, n, local_n;
	int my_rank, comm_sz;
	MPI_Comm comm;

	MPI_Init(NULL, NULL);
	comm = MPI_COMM_WORLD;
	MPI_Comm_size(comm, &comm_sz);
	MPI_Comm_rank(comm, &my_rank);

	Get_dims(&m, &local_m, &n, &local_n, my_rank, comm_sz, comm);

	MPI_Finalize();
	return 0;
}  /* main */
void Get_dims(
		int*      m_p        /* out */, 
		int*      local_m_p  /* out */,
		int*      n_p        /* out */,
		int*      local_n_p  /* out */,
		int       my_rank    /* in  */,
		int       comm_sz    /* in  */,
		MPI_Comm  comm       /* in  */) {

	if (my_rank == 0) {
		printf("Enter the number of rows\n");
		scanf("%d", m_p);
		printf("Enter the number of columns\n");
		scanf("%d", n_p);
	}
	MPI_Bcast(m_p, 1, MPI_INT, 0, comm);
	MPI_Bcast(n_p, 1, MPI_INT, 0, comm);
	*local_m_p = *m_p/comm_sz;
	*local_n_p = *n_p/comm_sz;
}  /* Get_dims */
