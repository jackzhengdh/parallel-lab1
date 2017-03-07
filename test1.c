#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// void Check_for_error(int local_ok, char fname[], char message[], 
// 	MPI_Comm comm);
// void Read_input(char filename[], int* n_p, int* local_n_p, doubel* err,
// 	int my_rank, int comm_sz, MPI_Comm comm);

void read_input(int* n_p, double* err_p, FILE* fp);
void read_matrix(int n, double x[], FILE* fp);

int main(int argc, char *argv[]) {
	
	// double* local_A;
	// double* local_x;
	// double* local_b;
	int n, local_n;
	double err;
	int my_rank, comm_sz;
	double* x;

	if(argc != 2)
	{
		printf("Incorrect number of arguments\n");
		exit(1);
	}


	char *filename = argv[1];
	printf("Check filename %s\n", filename);
	FILE* fp;
	// fp = fopen(argv[1], "r");
	fp = fopen(filename, "r");

	// fscanf(fp, "%d", &n);
	// fscanf(fp, "%lf", &err);
	read_input(&n, &err, fp);

	printf("test %d\n", n);
	printf("test %lf\n", err);
	x = malloc(n * sizeof(double));

	// read_input(&n, &err, fp);
	// printf("Read input n and err: %d%lf\n", n, err);
	read_matrix(n, x, fp);
	for (int i = 0; i < n; i++) {
		printf("%lf ", x[i]);
	}
	printf("\n");

	// MPI_Comm comm;

	// MPI_Init(NULL, NULL);
	// comm = MPI_COMM_WORLD;
	// MPI_Comm_size(comm, &comm_sz);
	// MPI_Comm_rank(comm, &my_rank);




}

void read_input(int* n_p, double* err_p, FILE* fp) {

	fscanf(fp, "%d ", n_p);
	fscanf(fp, "%lf ", err_p);

}

void read_matrix(int n, double x[], FILE* fp) {
	
	for (int i = 0; i < n; i++) {
		fscanf(fp, "%lf", &x[i]);
	}
}

// void Read_input(
// 	char		filename[] 	/* in */,
// 	int*		n_p 		/* out */, 
// 	int*		local_n_p 	/* out */,
// 	double*		err 		/* out */, 
// 	int 		my_rank 	/* in  */,
// 	int 		comm_sz 	/* in  */,
// 	MPI_Comm 	comm 		/* in  */) {

// 	int local_ok = 1;
// 	FILE* fp;
// 	fp = fopen(filename, "r");
// 	if (!fp) local_ok = 0;
// 	Check_for_error(local_ok, "Read_input", "Cannot open file", comm);

// 	if (my_rank == 0) {
// 		fscanf(fp, "%d", n_p);
// 		fscanf(fp, "%lf", err);
// 	}
// 	MPI_Bcast(n_p, 1, MPI_INT, 0, comm);
// 	MPI_Bcast(err, 1, MPI_INT, 0, comm);
// 	if (*n_p <= 0 || *n_p % comm_sz != 0 ) local_ok = 0;
// 	Check_for_error(local_ok, "Read_input",
// 		"number of variables must be positive and divisible by comm_sz",
// 		comm);
// 	*local_n_p = *n_p/comm_sz;

// }  /* Read_input */




