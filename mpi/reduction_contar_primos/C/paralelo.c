
#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>
#include<string.h>
#include <time.h>

#define _BLOCK_LOW(id, n_hilos, dimencion) ((id) * (dimencion) / (n_hilos))
#define _BLOCK_HIGH(id, n_hilos, dimencion) (_BLOCK_LOW((id) + 1, (n_hilos), (dimencion)) - 1)


int Es_primo(int numero);

void main (int argc,char *argv[] ) 
{
    double tiempo_inicio, tiempo_final;
    int dimencion = atoi(argv[1]);
    int procesadores=0;
    int resultado_global=0;

    double time_spent = 0.0;
    clock_t begin = clock();

    MPI_Init(NULL, NULL);    

        MPI_Status status;
        int id, fuente, destino, indice_local, limite_local;    
        int resultado = 0;

        MPI_Comm_size( MPI_COMM_WORLD, &procesadores ); // Numero total de procesos
        MPI_Comm_rank( MPI_COMM_WORLD, &id ); // Valor de nuestro identificador    
        
        indice_local = _BLOCK_LOW(id,procesadores, dimencion);
        limite_local = _BLOCK_HIGH(id,procesadores,dimencion);

        for(   ; indice_local <= limite_local; ++indice_local)
        {
            resultado += Es_primo(indice_local);
        }
        
         MPI_Reduce(&resultado, &resultado_global, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD); 

        if (id == 0)
        {
            clock_t end = clock();
            time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
            printf("%i ,%f ,%i \n",time_spent,procesadores, resultado_global);
        }

    MPI_Finalize(); 

    return ;
}


inline int Es_primo(int numero)
{
    
    int i=2;
	for (i; i * i  <= numero ; i++)
    {
        if (numero % i  == 0) 
        {
            return 0;
        } 
    };
    return 1;
}
