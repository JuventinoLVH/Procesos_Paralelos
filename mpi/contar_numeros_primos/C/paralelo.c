
#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>
#include<string.h>

#define _BLOCK_LOW(id, n_hilos, dimencion) ((id) * (dimencion) / (n_hilos))
#define _BLOCK_HIGH(id, n_hilos, dimencion) (_BLOCK_LOW((id) + 1, (n_hilos), (dimencion)) - 1)


int Es_primo(int numero);

void main (int argc,char *argv[] ) 
{
    double tiempo_inicio, tiempo_final;
    int dimencion = atoi(argv[1]);
    int procesadores;
    int resultado_root;

    MPI_Init(NULL, NULL);    

        tiempo_inicio = MPI_Wtime();

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
        
        // root = 0
        if(id != 0)
        {
            // puntero a la variable a mandar, longitud de la variable, tipo, ID destino, equiqueta , Recibidor
            MPI_Send( &resultado, 1, MPI_INT, 0, 5, MPI_COMM_WORLD);
        }
        else
        {
	    int i = 1;
            int resultado_externo;
            resultado_root = resultado;
            for(i ; i<procesadores;++i)
            {
                //Direccion Variable, Numero de datos a recivir , Tipo de dato recivido ,
                //  rank del emisor , Etiqueta , Comn\unicador, estatus <- Se puede ignorar por ahora    
                MPI_Recv( &resultado_externo, 1, MPI_INT, i, 5, MPI_COMM_WORLD, &status );
                resultado_root += resultado_externo;
            }
            tiempo_final = MPI_Wtime();              
            printf("%i ,%16g ,%i \n", procesadores, (tiempo_final - tiempo_inicio), resultado_root);
 
        }   
    
       
    MPI_Finalize(); 
    
    return ;
}


int Es_primo(int numero)
{
    if(numero < 2) return 0;

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
