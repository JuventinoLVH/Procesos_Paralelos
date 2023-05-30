#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include<string.h>
 
void main ( ) {
    MPI_Init(NULL, NULL);    
    int rank, size, fuente, destino;
    char mensaje[50];
    MPI_Status status;

    MPI_Comm_size( MPI_COMM_WORLD, &size ); // Numero total de procesos
    MPI_Comm_rank( MPI_COMM_WORLD, &rank ); // Valor de nuestro identificador    
    if(size>=2){
        fuente = (rank + size -1)%size;
        destino = (rank+1) %size;
        sprintf( mensaje, "Hola proceso %d un saludo desde el %d", destino,rank );

        MPI_Send( mensaje, strlen(mensaje) + 1, MPI_CHAR, destino, 5, MPI_COMM_WORLD);
        MPI_Recv( mensaje, 50, MPI_CHAR, fuente, 5, MPI_COMM_WORLD, &status );
        printf( "Soy %d, recibí <%s> desde %d\n", rank, mensaje, fuente);      
    
    }  
                         
    MPI_Finalize(); 
    return ;
}
