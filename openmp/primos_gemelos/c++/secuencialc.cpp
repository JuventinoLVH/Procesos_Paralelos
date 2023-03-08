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
#define _BLOCK_HIGH(id, h, n) (_BLOCK_LOW((id) + 1, (h), (n)) - 1)
#define _BLOCK_SIZE(id, h, n) (_BLOCK_HIGH((id), (h), (n)) - _BLOCK_LOW((id), (h), (n)) + 1)
#define _MAX(i, j) (((i) > (j)) ? (i) : (j))

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
    
    double t_ini,t_fin,t_seccion1=0,t_seccion2=0;
    // Se crea un arreglo
    int dimension = atoi(argv[1]);
    int repeticiones = atoi(argv[2]);    
      
    bool *numeros_marcados;
    numeros_marcados = new bool[dimension+1];
    if (numeros_marcados == NULL) {
        cout<<"Cannot allocate enough memory\n";
        exit (1);
    }


    t_ini = omp_get_wtime();

    for(int rep = 0 ; rep< repeticiones; ++rep)
    {
        int numero_primos_gemelos=0;
        int indice_criba = 2;      
        int id = 0;
        int indice_local = 0;
        int limite_local = dimension;
        // Marcamos a todos los numeros, asumimos que todos los numeros son primos.
        for(int i = indice_local ;  i <= limite_local ; ++i)
        {
            numeros_marcados[i] = true;
        }
        numeros_marcados[0] = false;
        numeros_marcados[1] = false;


        // Empieza la criba
        int sangria_primo_local = 0;
        do
        {
            // marcando a los multiplos del numero primo
            sangria_primo_local = indice_criba * indice_criba - indice_local;

            // Se busca el siguiente primo   
            for (int i = sangria_primo_local + indice_local ; i <= limite_local; i += indice_criba )
            {
                numeros_marcados[i] = false;
            }
                        

            // Se busca el siguiente indice
            while(indice_criba < dimension && !numeros_marcados[++indice_criba]){}; 
        }while (indice_criba * indice_criba  <= dimension);
        

        // Calculo del numero de primos gemelos
        for(int i=3; i<= dimension; ++i)
        {
            if(numeros_marcados[i] && numeros_marcados[i-2])
                ++numero_primos_gemelos;
        }
    }       
    t_fin = omp_get_wtime();
    printf("1 ,%16g \n", (t_fin - t_ini)/repeticiones);
    // Imprimir la informacion
    //cout<<"Criba hasta el numero: "<<dimension<<'\n';
    //cout<<"Mayor gap: "<<numero_primos_gemelos<<'\n';
    //Imprimir_Primos(numeros_marcados,dimension);
    delete [] numeros_marcados;
    return 0;
}