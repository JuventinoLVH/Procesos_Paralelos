/*
    Autor: Luis Juventino Velasquez Hidalgo
    Fecha: 13/02/23
    Descripcion: implementacion de la Criba de Aristoteles con procesamiento 
        paralelo, la finalidad es ver que tanto es el speedUp en relacion al modelo secuencial  
*/
#include <iostream>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define _BLOCK_LOW(id, n_hilos, dimension) ((id) * (dimension) / (n_hilos))
#define _BLOCK_HIGH(id, n_hilos, dimension) (_BLOCK_LOW((id) + 1, (n_hilos), (dimension)) - 1)
#define _BLOCK_SIZE(id, n_hilos, dimension) (_BLOCK_HIGH((id), (n_hilos), (dimension)) - _BLOCK_LOW((id), (n_hilos), (dimension)) + 1)
#define _MAX(i, j) (((i) > (j)) ? (i) : (j))
#define _MIN(i, j) (((i) < (j)) ? (i) : (j))

using namespace std;

void Imprimir_Primos(bool numeros_marcados[], int &n){
    cout<<"PRIMOS = [";
    #pragma omp for ordered
    for (int i = 2; i <= n; i++)
        if (numeros_marcados[i])
            cout<<i<<", ";
    cout<<"\b\b]\n";
}


int main(int argc, char *argv[])
{
    
    double mejor_tiempo=999,t_ini,t_fin,t_seccion1=0,t_seccion2=0;
    // Se crea un arreglo
    int dimension = atoi(argv[1]);
    int numero_hilos = atoi(argv[2]);
    int repeticiones = atoi(argv[3]);

      
    bool *numeros_marcados;
    numeros_marcados = new bool[dimension+1];
    if (numeros_marcados == NULL) {
        cout<<"Cannot allocate enough memory\n";
        printf("%i ,%16g \n", numero_hilos, 0);
        exit (1);
    }

    for(int repeticion =0;repeticion < repeticiones;++repeticion)
    {
        t_ini = omp_get_wtime();
        int primos_gemelos=0;
        int indice_criba=2;      
        for(int rep = 0 ; rep< repeticiones; ++rep)
        {
            #pragma omp parallel num_threads(numero_hilos) \
                shared(numeros_marcados,dimension, indice_criba,primos_gemelos)
            {
                indice_criba=2; 
                primos_gemelos=0;
                //Primera region paralela
                // Marcamos a todos los numeros, asumimos que todos los numeros son primos.
                int id = omp_get_thread_num();
                int indice_local = _BLOCK_LOW(id,numero_hilos,dimension);
                int limite_local = _BLOCK_HIGH(id,numero_hilos,dimension);
                

                for(int i = indice_local ;  i <= limite_local ; ++i)
                {
                    numeros_marcados[i] = true;
                }
                numeros_marcados[0] = false;
                numeros_marcados[1] = false;


                // Segunda region paralela
                // Empieza la criba
                #pragma omp barrier
                int sangria_primo_local = 0;
                do
                {
                    //Segunda seccion paralela, primera parte
                    // marcando a los multiplos del numero primo
                    if ( indice_criba * indice_criba > indice_local )
                        sangria_primo_local = indice_criba * indice_criba - indice_local;
                    else {
                        if ( !(indice_local % indice_criba) ) 
                            sangria_primo_local = 0;
                        else 
                            sangria_primo_local = indice_criba - (indice_local % indice_criba);
                    }
                    
                    for (int i = sangria_primo_local + indice_local ; i <= limite_local; i += indice_criba )
                    {
                        numeros_marcados[i] = false;
                    }
                            

                    // segunda seccion paralela parte 2
                    // Se busca el siguiente indice
                    #pragma omp barrier
                    if(id == 0)
                    {
                        while(indice_criba < dimension && !numeros_marcados[indice_criba])
                        {indice_criba=+2;}; 
                    }

                    #pragma omp barrier
                }while (indice_criba * indice_criba  <= dimension);
                

                // Tercera parte paralela
                // Calculo de los numeros primos gemelos            
                #pragma omp barrier
                if(id == 1)
                {
                    indice_local = 2;
                } 
                    

                for( int index = indice_local ; index <= limite_local; ++index)
                {
                    if(numeros_marcados[index] && numeros_marcados[index-2])
                    {
                        #pragma omp atomic
                        ++primos_gemelos;
                    }    
                }
            } //Fin seccion paralela       
        }
        t_fin = omp_get_wtime();
        mejor_tiempo = _MIN((t_fin - t_ini),mejor_tiempo);
    }
    printf("%i ,%16g \n", numero_hilos, mejor_tiempo);
    delete [] numeros_marcados;
    return 0;
}