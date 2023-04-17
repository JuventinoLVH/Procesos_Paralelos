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
 
        fuente = rank/2;
        destino = rank*2;
        sprintf( mensaje, "Hola proceso %d un saludo desde el %d", destino,rank );

        if(rank < size/2 + size%2)
            MPI_Send( mensaje, strlen(mensaje) + 1, MPI_CHAR, 2*rank, 5, MPI_COMM_WORLD);
        
        if(rank%2 == 0)
        {
            MPI_Recv( mensaje, 50, MPI_CHAR, fuente, 5, MPI_COMM_WORLD, &status );
            printf( "Soy %d, recibí <%s> desde %d\n", rank, mensaje, fuente);      
        }
    }  
                         
    MPI_Finalize(); 
    return ;
}
