/*
 *   Sieve of Eratosthenes
 *
 *   This MPI program computes the number of prime numbers
 *   less than N, where N is a command-line argument.
 *
 *   Enhancements:
 *      Only odd integers are represented
 *      Each process finds its own prime sieve values: there
 *         is no broadcast step
 *
 *   Programmer: Michael J. Quinn
 *
 *   Last modification: 6 September 2001
 */

#include "mpi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define MIN(a,b)  ((a)<(b)?(a):(b))

int main (int argc, char *argv[]){
    int    current_prime;     /* Sieve by this prime */
    double elapsed_time;      /* Stopwatch */
    int    id;            /* Process rank */
    int    n;                 /* Top integer to check */
    int    p;                 /* Number of processors */
    int    sqrt_n;            /* Top value for sieve primes */
    char  *small_primes;      /* 1's show primes to sqrt(n) */
    char  *primes;            /* Share of values 3..n */
    int    small_prime_count; /* Number of sieve primes */
    int   *small_prime_values;/* Array of sieving primes */
    int    index;             /* Sieving location */
    int    i, j, k;
    int    size;              /* Size of array 'primes' */
    int    prime_count;       /* Primes on this proc */
    int    low_proc_value;    /* Smallest int on this proc */
    int    high_proc_value;   /* Highest int on this proc */
    int    smaller_size;      /* Smaller block size */
    int    larger_size;
    int    num_larger_blocks;
    int blocks;
    int low;
    int high;
    int global_count;
    int small_prime_array_size;
    int els;
    
    int seq_sieve (char *, int , int );

    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &id);
    MPI_Comm_size (MPI_COMM_WORLD, &p);

    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time = -MPI_Wtime();
    n = atoi (argv[1]);
    //printf("CRIBA versión3\n ");
    sqrt_n = (int) sqrt((double) n);
    small_prime_array_size = (sqrt_n - 1)/2;
    small_primes = (char *) malloc (small_prime_array_size);
    if (small_primes == NULL) {
        printf ("Cannot allocate enough memory (small_primes)\n");
        MPI_Finalize();
        exit (1);
    }

    small_prime_count = seq_sieve (small_primes, small_prime_array_size, sqrt_n);
    small_prime_values = (int *) malloc (small_prime_count * sizeof(int));
    if (small_prime_values == NULL) {
        printf ("Cannot allocate enough memory (small_prime_values)\n");
        MPI_Finalize();
        exit (1);
    }
    index = 0;
    for (i = 0; i < small_prime_array_size; i++)
        if (small_primes[i]) 
            small_prime_values[index++] = 2*i+3;
        
    els = (n-1) / 2;
    smaller_size = els / p;
    larger_size = smaller_size + 1;
    num_larger_blocks = els % p;
    if (id < num_larger_blocks) 
        size = larger_size;
    else 
        size = smaller_size;
    
    low_proc_value  = 2*(id*smaller_size + MIN(id, num_larger_blocks)) + 3;
    high_proc_value = low_proc_value + 2 * (size-1);
    
    primes = (char *) malloc (size);
    if (primes == NULL) {
        printf ("Cannot allocate enough memory (primes)\n");
        MPI_Finalize();
        exit (1);
    }
    for (j = 0; j < size; j++) primes[j] = 1;

    for (j = 0; j < small_prime_count; j++) {
        current_prime = small_prime_values[j];
        if (current_prime * current_prime > low_proc_value)
            index = (current_prime * current_prime - low_proc_value)/2;
        else {
            int r = low_proc_value % current_prime;
            if (!r) 
                index = 0;
            else 
            if ((current_prime - r) & 1)
                index = (2*current_prime - r)/2;
            else 
                index = (current_prime - r)/2;
        }
        if (index >= size) break;
        for (k = index; k < size; k+= current_prime)
            primes[k] = 0;
    }

    prime_count = 0;
    int prim, startPrim = 0, endPrim = 0, startRec = 0;
    int flagStart = 0;
    for (j = 0; j < size; j++){
        if (primes[j]) {
            if (!flagStart){
                flagStart = 1;
                startPrim = low_proc_value + 2*j;
            }
            if(j-1 >= 0 && j+1 < size && primes[j-1] && primes[j+1]) prime_count++;
            else if(j+1 < size && primes[j+1]) prime_count+=2;
  
            endPrim = low_proc_value + 2*j;
        }
      }
    MPI_Barrier(MPI_COMM_WORLD);

    if(id != 0 ){
      MPI_Send(&startPrim, 1, MPI_INT, id-1, 9, MPI_COMM_WORLD);
    }

    if(id != p-1){
      MPI_Recv(&startRec, 1, MPI_INT, id+1, 9, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      if (startRec != 0 && endPrim != 0 && startRec-endPrim == 2)prime_count+=2;
    }
    
    MPI_Reduce (&prime_count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);    

    free(primes);
    free(small_primes);
    free(small_prime_values);

    elapsed_time += MPI_Wtime();

    if (!id) {
        printf (" %d, %d,  %10.6f \n", p,global_count, elapsed_time);
    }
    MPI_Finalize();
    return 0;
}
//--------------------------------------------------------------------
int seq_sieve (char *small_primes, int small_prime_array_size, int sqrt_n)
{
    int i, j;
    int prime_index;
    int prime_value;
    int count;

    /* small_primes[i] represents integer 2i+3 */

    for (i = 0; i < small_prime_array_size; i++) 
        small_primes[i] = 1;

    prime_index = 0;
    prime_value = 3;
    while (prime_value * prime_value <= sqrt_n) {
        j = prime_value * prime_value / 2 - 1;
        while (j < small_prime_array_size) {
            small_primes[j] = 0;
            j += prime_value;
        }
        while (small_primes[++prime_index] == 0);
        prime_value = 2*prime_index + 3;
    }
    count = 0;
    for (i = 0; i < small_prime_array_size; i++)
        if (small_primes[i] == 1)
            count++;
        
    return count;
}
