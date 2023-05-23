#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
// -----------------------------------------------------------------------------
float time_diff(struct timeval *start, struct timeval *end){
    return (end->tv_sec - start->tv_sec) + 1e-6*(end->tv_usec - start->tv_usec);
}
// -----------------------------------------------------------------------------
int es_primo(int m){
    int i,ind;
    ind=1;
    for(i=2;i<=sqrt(m) && ind==1;i++)
        if(m % i==0){
            ind=0;
            break;
        }
    return ind;
}
// -----------------------------------------------------------------------------
#define BLOCK_LOW(id, p, n) ((id) * (n) / (p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id) + 1, p, n) - 1)
#define BLOCK_SIZE(id, p, n) (BLOCK_HIGH(id, p, n) - BLOCK_LOW(id, p, n) + 1)
// -----------------------------------------------------------------------------
int main(int argc, char **argv){
    MPI_Init(&argc, &argv);
    int id, i,  np, n, aux, inicio, fin, numPorRango, root = 0, total;
    double t1, t2;

    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    if(argc!=2){
        if(id==0) printf("se requiere que se envien 2 argumentos\n"); 
        MPI_Finalize();       
        return 1;
    }

    t1 = MPI_Wtime(); // <----- tiempo1
    n = atoi(argv[1]);
    inicio = BLOCK_LOW(id, np, n) + 1; // para que empiece en 1    
    fin    = BLOCK_HIGH(id, np, n) + 1;
    numPorRango = BLOCK_SIZE(id, np, n);

    if(id==0) inicio=2;

    int cont = 0;
    MPI_Barrier(MPI_COMM_WORLD);
    
    
    for (i = inicio; i <= fin; i++)
        cont += es_primo(i);
        
    MPI_Reduce(&cont, &total, 1, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD);  
    t2 = MPI_Wtime();
    if (id == root) {
        float tiempo = t2-t1;
        printf("%d, %f, %d\n", np, tiempo, total);
    }
    
    MPI_Finalize();
    return 0;
}

