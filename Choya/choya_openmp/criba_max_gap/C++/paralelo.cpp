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

#define MAX(i, j) (((i) > (j)) ? (i) : (j))

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
    
    double t_ini,t_fin;
    // Se crea un arreglo
    int dimension = atoi(argv[1]);
    int numero_hilos = atoi(argv[2]);

    bool *numeros_marcados;
    numeros_marcados = new bool[dimension+1];
    if (numeros_marcados == NULL) {
        cout<<"Cannot allocate enough memory\n";
        exit (1);
    }
    numeros_marcados[0] = numeros_marcados[1] = false;


    int maximo_gap=1;
    int total_primos = 0;
    int tamano_chunk = ceil(dimension / numero_hilos);
    int indice_criba = 2;
    int ultimo_primo = 2;
    int multiplos_aprox = 0;
    int iterasciones_asignadas = 0; 
    int primo_anterior = 0;
    
    t_ini = omp_get_wtime();  //La barrera que hay en la criba hace muy lento la ejecuccion
    #pragma omp parallel num_threads(numero_hilos) \
        shared(total_primos,numeros_marcados,dimension,tamano_chunk, \
        indice_criba,multiplos_aprox,iterasciones_asignadas,ultimo_primo) \
        reduction(max : maximo_gap) private(primo_anterior)
    {
        int id = omp_get_thread_num();
        int indice_local = id * tamano_chunk;
        int limite_local = (id+1) * tamano_chunk ;
        // Marcamos a todos los numeros, asumimos que todos los numeros son primos.
        for(int i = indice_local ; i <= dimension && i < limite_local ; ++i){
            numeros_marcados[i] = true;
        }
        
        if(id == 0)
        {
            tamano_chunk = ceil((dimension - ultimo_primo) / numero_hilos);
            numeros_marcados[0] = false;
            numeros_marcados[1] = false;
            multiplos_aprox = ceil((dimension - ultimo_primo ) / ultimo_primo);
            iterasciones_asignadas = ceil( multiplos_aprox / numero_hilos);
        }
//sincronizada, pues la sincronicacion solo ocurre cuando se encuentra un nuevo numero primo
//      y el primer primo es solamente el 2
        #pragma omp barrier

        // Empieza la criba
        do
        {
            // marcando a los multiplos del numero primo
            if(ultimo_primo <= dimension/2)
            { 
                indice_local = ultimo_primo + id*iterasciones_asignadas;
                limite_local = ultimo_primo + (id+1)*iterasciones_asignadas;

                for (int i = indice_local ; i <= dimension && i <= limite_local; i += ultimo_primo)
                {
                    numeros_marcados[i] = false;
                }
            }          

            #pragma omp barrier
//Synconicacion, el hilo 0 espera a que todos terminen su parte.
            
            if(id == 0)
            {
                while(indice_criba < dimension && !numeros_marcados[++indice_criba]){}; \

                ultimo_primo = indice_criba;
                multiplos_aprox = ceil((dimension - ultimo_primo ) / ultimo_primo);
                iterasciones_asignadas = ceil( multiplos_aprox / numero_hilos);
            }

            #pragma omp barrier
//Synconicacion, todos los hilos esperan a que el indice 0 busque al siguiente primo 
        }while (indice_criba < dimension);
        
        
        #pragma omp barrier
        //t_ini = omp_get_wtime();
//Synconicacion, a partir de aqui todos los hilos van igual
        //Contamos el numero de primos y el maximo gap  
        
        tamano_chunk = ceil(dimension / numero_hilos);
        indice_local = id * tamano_chunk;
        limite_local = (id+1) * tamano_chunk ;

        int primo_anterior =indice_local;
        while(primo_anterior <= dimension && !numeros_marcados[primo_anterior]) ++primo_anterior; 

        for ( indice_local = primo_anterior ;
                primo_anterior < limite_local  && indice_local <= dimension;
                ++indice_local) 
        {
            if(numeros_marcados[indice_local])
            {
                maximo_gap = max(maximo_gap ,indice_local-primo_anterior);
                primo_anterior = indice_local;
            }
        }
            
    }
    t_fin = omp_get_wtime();
    printf("%i ,%16g \n",numero_hilos,(t_fin - t_ini));
    // Imprimir la informacion
    //cout<<"Criba hasta el numero: "<<dimension<<'\n';
    //cout<<"Mayor gap: "<<maximo_gap<<'\n';
    //Imprimir_Primos(numeros_marcados,dimension);
    delete [] numeros_marcados;
    return 0;
}