/*
 *   Sieve of Eratosthenes
 *
 *   This MPI program computes the number of prime numbers less than N, where
 *   N is a command-line argument.
 *
 *   Enhancements:
 *      Only odd integers are represented
 *      Each process finds its own prime sieve values: no broadcast step
 *      Large array considered one cache block at a time to improve hit rate
 *
 *   Programmer: Michael J. Quinn
 *
 *   Last modification: 6 September 2001
 */

#include "mpi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define MIN(a,b)   ((a)<(b)?(a):(b))
#define BLOCK_SIZE 15000
#define _MAX(x,y)( x > y ? x : y)

int seq_sieve (char *small_primes, int small_prime_array_size, int sqrt_n)
{
    int i, j;
    int prime_index;
    int prime_value;
    int count;

    /* small_primes[i] represents integer 2i+3 */

    for (i = 0; i < small_prime_array_size; i++) small_primes[i] = 1;
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
// -----------------------------------
int main (int argc, char *argv[]){
    int blocks;                 /* Number of blocks in subarray */
    int current_prime;          /* Prime currently used as sieve value */
    double elapsed_time;        /* Elapsed wall clock time */
    int els;                    /* Global total of elements in 'primes' */
    int global_count;           /* Total number of primes up through n */
    int high_block_index;       /* Last index of block being sieved */
    int high_block_value;       /* Value of last element of block being sieved */
    int high_proc_value;        /* Integer represented by last el in 'primes' */
    int i;
    int id;                     /* Process ID number */
    int index;                  /* Location of first multiple of 'current_prime'
                                    in array 'primes' */
    int j;
    int k;
    int larger_size;            /* Larger subarray size */
    int low_block_index;        /* First index of block being sieved */
    int low_block_value;        /* Value of 1st element of block being sieved */
    int low_proc_value;         /* Integer represented by 1st el in 'primes' */
    int n;                      /* Upper limit of sieve */
    int num_larger_blocks;      /* Number of processes with larger subarrays */
    int p;                      /* Number of processes */
    int prime_count;            /* Number of primes in 'prime' */
    char *primes;               /* Process's portion of integers 3,5,...,n */
    int size;                   /* Number of elements in 'primes' */
    int small_prime_array_size; /* Number of elements in 'small_primes' */
    int small_prime_count;      /* Number of odd primes through sqrt(n) */
    int *small_prime_values;    /* List of odd primes up to sqrt(n) */
    char *small_primes;         /* Used to sieve odd primes up to sqrt(n) */
    int smaller_size;           /* Smaller subarray size */
    int sqrt_n;                 /* Square root of n, rounded down */
    
    MPI_Status status;
    
    int fuente, destino, indice_local, limite_local;
    int primo_anterior,primer_elemento_primo=-1;
    int primos_cont=0;    
    int primos=0;
    char siguiente_es_primo;

    
    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &id);
    MPI_Comm_size (MPI_COMM_WORLD, &p);

    /* Start timer */
    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time = -MPI_Wtime();
    n = atoi (argv[1]);

    sqrt_n = (int) sqrt((double) n);
    small_prime_array_size = (sqrt_n - 1)/2;
    small_primes = (char *) malloc (small_prime_array_size);
    if (small_primes == NULL) {
        printf ("No hay memoria suficiente (small_primes)\n");
        MPI_Finalize();
        exit (1);
    }
    small_prime_count = seq_sieve (small_primes, small_prime_array_size, sqrt_n);
    small_prime_values = (int *) malloc (small_prime_count * sizeof(int));
    if (small_prime_values == NULL) {
        printf ("No hay memoria suficiente (small_prime_values)\n");
        MPI_Finalize();
        exit (1);
    }
    j = 0;
    for (i = 0; i < small_prime_array_size; i++)
        if (small_primes[i])
            small_prime_values[j++] = 2*i+3;
        
    
    els = (n-1) / 2;
    smaller_size = els / p;
    larger_size = smaller_size + 1;
    num_larger_blocks = els % p;
    if (id < num_larger_blocks) 
        size = larger_size;
    else 
        size = smaller_size;

    low_proc_value = 2*(id*smaller_size + MIN(id, num_larger_blocks)) + 3;
    high_proc_value = low_proc_value + 2 * (size-1);
    primes = (char *) malloc (size);
    if (primes == NULL) {
        printf ("No hay memoria suficiente (primes)\n");
        MPI_Finalize();
        exit (1);
    }

    blocks = size / BLOCK_SIZE;
    if (BLOCK_SIZE * blocks < size) blocks++;
    prime_count = 0;
    for (i = 0; i < blocks; i++) {
        low_block_index = i * BLOCK_SIZE;
        high_block_index = ((i + 1) * BLOCK_SIZE) - 1;
        if (high_block_index > size-1) high_block_index = size-1;
        for (j = low_block_index; j <= high_block_index; j++) primes[j] = 1;
        low_block_value = low_proc_value + 2*low_block_index;
        high_block_value = low_proc_value + 2*high_block_index;
        for (j = 0; j < small_prime_count; j++) {
            current_prime = small_prime_values[j];
            if (current_prime * current_prime > low_block_value)
                index = low_block_index + (current_prime * current_prime - low_block_value)/2;
            else {
                int r = low_block_value % current_prime;
                if (!r) index = low_block_index;
                else 
                    if ((current_prime - r) & 1)
                        index = low_block_index + (2*current_prime - r)/2;
                    else index = low_block_index + (current_prime - r)/2; 
            }
            if (index > high_block_index) break;
            for (k = index; k <= high_block_index; k+= current_prime)
                primes[k] = 0;
        }
    }
        

/*#############################################################################
  ########################### Aqui empieza mi codigo ##########################
  ###########################################################################*/
    int iterador_local = 1;
    
    //----------------------------------------------------------------------------
    /* Calculamos el conteo de primos gemelos,ya que el algoritmo salta de dos 
        en dos solo hay que revisar si el elemento anterior es primo
    */
    for (iterador_local; iterador_local < size; ++iterador_local)
    {
        primos = ((primes[iterador_local] && primes[iterador_local-1]) ? primos+1 : primos );
    }
    //----------------------------------------------------------------------------
    /* NO encontramos el primer primo. solo preguntamos si el primer elemento 
        lo es. En todos los demas casos el resultado no cambia al momento
        de calcular un primo gemelo que esta 'entre dos procesos'
    */
    if(id != 0)
    {
        // puntero a la variable a mandar, longitud de la variable, tipo, ID destino, equiqueta , Recibidor
        MPI_Send( &primes[0], 1, MPI_CHAR, id-1, 5, MPI_COMM_WORLD);
    }
    //----------------------------------------------------------------------------
    /* Se revisa si el 'puente' entre dos bloques es primo_gemelos 
    */
    if(id != p-1 )
    {
        MPI_Recv( &siguiente_es_primo, 1, MPI_CHAR, id+1, 5, MPI_COMM_WORLD, &status );
        primos = ((primes[size-1] && siguiente_es_primo)? primos+1 : primos );
    }

    // Reduce de cajon
    MPI_Reduce(&primos, &primos_cont, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD); 
/*#############################################################################
  ########################### Aqui termina mi codigo ##########################
  ###########################################################################*/
      
    free(primes);
    free(small_primes);
    free(small_prime_values);

    elapsed_time += MPI_Wtime();
    if (!id) {
        //printf ("Total prime count is %d\n", global_count);
        printf (" %d, %10.6f, %d\n", p, elapsed_time,primos_cont);
    }
    
    MPI_Finalize();
    return 0;
}
// ---------------------------------------------------

