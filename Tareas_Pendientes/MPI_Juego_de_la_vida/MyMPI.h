/*   MyMPI.h
 *
 *   Header file for a library of matrix/vector
 *   input/output/redistribution functions.
 *
 *   Programmed by Michael J. Quinn
 *
 *   Last modification: 4 September 2002
 */

/************************* MACROS **************************/

#define DATA_MSG           0
#define PROMPT_MSG         1
#define RESPONSE_MSG       2

#define OPEN_FILE_ERROR    -1
#define MALLOC_ERROR       -2
#define TYPE_ERROR         -3

#define MIN(a,b)           ((a)<(b)?(a):(b))
#define BLOCK_LOW(id,p,n)  ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id)+1,p,n)-1)
#define BLOCK_SIZE(id,p,n) (BLOCK_HIGH(id,p,n)-BLOCK_LOW(id,p,n)+1)
#define BLOCK_OWNER(j,p,n) (((p)*((j)+1)-1)/(n))
#define PTR_SIZE           (sizeof(void*))
#define CEILING(i,j)       (((i)+(j)-1)/(j))

/***************** MISCELLANEOUS FUNCTIONS *****************/
void  terminate (int, char *);
/****************** INPUT FUNCTIONS ************************/
void read_row_striped_matrix (char *, void ***, void **,        MPI_Datatype, int *, int *, MPI_Comm);
/****************** OUTPUT FUNCTIONS ***********************/

void print_game(void **, MPI_Datatype, int,        int, MPI_Comm);

/*
 *   Given MPI_Datatype 't', function 'get_size' returns the
 *   size of a single datum of that data type.
 */
//  ■■■■■■■■■■■■■■■■■
int get_size (MPI_Datatype t) {
    if (t == MPI_BYTE) return sizeof(char);
    if (t == MPI_DOUBLE) return sizeof(double);
    if (t == MPI_FLOAT) return sizeof(float);
    if (t == MPI_INT) return sizeof(int);
    printf ("Error: Unrecognized argument to 'get_size'\n");
    fflush (stdout);
    MPI_Abort (MPI_COMM_WORLD, TYPE_ERROR);

    return 0;
}


/*
 *   Function 'my_malloc' is called when a process wants
 *   to allocate some space from the heap. If the memory
 *   allocation fails, the process prints an error message
 *   and then aborts execution of the program.
 */
//  ■■■■■■■■■■■■■■■■■
void *my_malloc (
   int id,     /* IN - Process rank */
   int bytes)  /* IN - Bytes to allocate */
{
    void *buffer;
    if ((buffer = malloc ((size_t) bytes)) == NULL) {
        printf ("Error: Malloc failed for process %d\n", id);
        fflush (stdout);
        MPI_Abort (MPI_COMM_WORLD, MALLOC_ERROR);
    }
    return buffer;
}


/*
 *   Function 'terminate' is called when the program should
 *   not continue execution, due to an error condition that
 *   all of the processes are aware of. Process 0 prints the
 *   error message passed as an argument to the function.
 *
 *   All processes must invoke this function together!
 */
//  ■■■■■■■■■■■■■■■■■
void terminate (
   int   id,            /* IN - Process rank */
   char *error_message) /* IN - Message to print */
{
    if (!id) {
        printf ("Error: %s\n", error_message);
        fflush (stdout);
    }
    MPI_Finalize();
    exit (-1);
}



/*
 *   Process p-1 opens a file and inputs a two-dimensional
 *   matrix, reading and distributing blocks of rows to the
 *   other processes.
 */
//  ■■■■■■■■■■■■■■■■■■■■■■■■
void read_row_striped_matrix (
   char        *s,        /* IN - File name */
   void      ***subs,     /* OUT - 2D submatrix indices */
   void       **storage,  /* OUT - Submatrix stored here */
   MPI_Datatype dtype,    /* IN - Matrix element type */
   int         *m,        /* OUT - Matrix rows */
   int         *n,        /* OUT - Matrix cols */
   MPI_Comm     comm)     /* IN - Communicator */
{
    int          datum_size;   /* Size of matrix element */
    int          i;
    int          id;           /* Process rank */
    FILE        *infileptr;    /* Input file pointer */
    int          local_rows;   /* Rows on this proc */
    void       **lptr;         /* Pointer into 'subs' */
    int          p;            /* Number of processes */
    void        *rptr;         /* Pointer into 'storage' */
    MPI_Status   status;       /* Result of receive */
    int          x;            /* Result of read */

    MPI_Comm_size (comm, &p);
    MPI_Comm_rank (comm, &id);
    datum_size = get_size (dtype);

    /* Process p-1 opens file, reads size of matrix,
        and broadcasts matrix dimensions to other procs */

    if (id == (p-1)) {
        infileptr = fopen (s, "r");
        if (infileptr == NULL) *m = 0;
        else {
            fread (m, sizeof(int), 1, infileptr);
            fread (n, sizeof(int), 1, infileptr);
        }      
    }
    MPI_Bcast (m, 1, MPI_INT, p-1, comm);

    if (!(*m)) MPI_Abort (MPI_COMM_WORLD, OPEN_FILE_ERROR);

    MPI_Bcast (n, 1, MPI_INT, p-1, comm);

    local_rows = BLOCK_SIZE(id,p,*m);

    /* Dynamically allocate matrix. Allow double subscripting
        through 'a'. */

    *storage = (void *) my_malloc (id, local_rows * *n * datum_size);
    *subs = (void **) my_malloc (id, local_rows * PTR_SIZE);

    lptr = (void *) &(*subs[0]);
    rptr = (void *) *storage;
    for (i = 0; i < local_rows; i++) {
        *(lptr++)= (void *) rptr;
        rptr += *n * datum_size;
    }

    /* Process p-1 reads blocks of rows from file and
        sends each block to the correct destination process.
        The last block it keeps. */

    if (id == (p-1)) {
        for (i = 0; i < p-1; i++) {
            x = fread (*storage, datum_size, BLOCK_SIZE(i,p,*m) * *n, infileptr);
            MPI_Send (*storage, BLOCK_SIZE(i,p,*m) * *n, dtype, i, DATA_MSG, comm);
        }
        x = fread (*storage, datum_size, local_rows * *n,  infileptr);
        fclose (infileptr);
    } else
        MPI_Recv (*storage, local_rows * *n, dtype, p-1, DATA_MSG, comm, &status);
}


/******************** OUTPUT FUNCTIONS ********************/
void print_gameSubmatrix(
   void       **a,       /* OUT - Doubly-subscripted array */
   MPI_Datatype dtype,   /* OUT - Type of array elements */
   int          rows,    /* OUT - Matrix rows */
   int          cols)    /* OUT - Matrix cols */
{
    int i, j;

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            if (dtype == MPI_DOUBLE)
                printf ("%6.3f ", ((double **)a)[i][j]);
            else{
                if (dtype == MPI_FLOAT)
                    printf ("%6.3f ", ((float **)a)[i][j]);
                else if (dtype == MPI_INT)
                    if(((int **)a)[i][j] == 0) printf("[ ]");
                    else printf("[■]");
                    //printf ("%12d", ((int **)a)[i][j]);
            }
        }
        putchar ('\n');
    }
}

void print_game(
	void **a,		/* IN - 2D array */
	MPI_Datatype dtype,	/* IN - matrix element type */
	int m,			/* IN - matrix rows */
	int n,			/* IN - matrix columns */
	MPI_Comm comm)		/* IN - communicator */
{
	MPI_Status	status;
	void		*bstorage;
	void		**b;
	int		datum_size;
	int		i;
	int		id;
	int		local_rows;
	int		max_block_size;
	int		prompt;
	int		p;

	MPI_Comm_rank(comm, &id);
	MPI_Comm_size(comm, &p);
	local_rows = BLOCK_SIZE(id,p,m);
	
	if (!id) {
		print_gameSubmatrix(a, dtype, local_rows, n);
		if (p > 1) {
			datum_size = get_size(dtype);
			max_block_size = BLOCK_SIZE(p-1,p,m);
			bstorage = my_malloc(id, max_block_size*n*datum_size);
			b = (void**)my_malloc(id, max_block_size*sizeof(void*));
			b[0] = bstorage;
			for (i = 1; i < max_block_size; i++) {
				b[i] = b[i-1] + n*datum_size;
			}
			for (i = 1; i < p; i++) {
				MPI_Send(&prompt, 1, MPI_INT, i, PROMPT_MSG, MPI_COMM_WORLD);
				MPI_Recv(bstorage, BLOCK_SIZE(i,p,m)*n, dtype, i, RESPONSE_MSG, MPI_COMM_WORLD, &status);
				print_gameSubmatrix(b, dtype, BLOCK_SIZE(i,p,m), n);
			}
			free(b);
			free(bstorage);
		}
		putchar('\n');
	}
	else {
		MPI_Recv(&prompt, 1, MPI_INT, 0, PROMPT_MSG, MPI_COMM_WORLD, &status);
		MPI_Send(*a, local_rows*n, dtype, 0, RESPONSE_MSG,	MPI_COMM_WORLD);
	}
}