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
    
    t_ini = omp_get_wtime();
    // Marcamos a todos los numeros, asumimos que todos los numeros son primos.
    #pragma omp parallel for num_threads(numero_hilos) shared(numeros_marcados,dimension)
        for(int i = 2 ; i <= dimension ; i++ ){
            numeros_marcados[i] = true;
        }

    // Empezamos a marcar desde el 2, el primer primo.    
    int indice = 2,ultimo_primo = 2;
    do
    {
        // marcando a los multiplos del numero primo
        #pragma omp parallel for num_threads(numero_hilos) shared(indice,ultimo_primo,numeros_marcados)
            for (int i = indice + ultimo_primo ; i <= dimension; i += ultimo_primo)
                numeros_marcados[i] = false;

        //Nota, aqui hay una barrera implicita, el hilo en single espera 
        //a que se terimne la ejecucion del ciclo 

        // buscando el siguiente valor sin marcar
        while(indice < dimension && !numeros_marcados[++indice]); 
        ultimo_primo = indice;
    }while (indice < dimension);

        
     //Contamos el numero de primos y el maximo gap
    int maximo_gap=1;
    int total_primos = 0;
    int tamano_chunk = (dimension / numero_hilos)+1;
    
    #pragma omp parallel num_threads(numero_hilos) \
        shared(total_primos,numeros_marcados,dimension,tamano_chunk) \
        reduction(max : maximo_gap) private(ultimo_primo)
    {
        int id = omp_get_thread_num();
        int indice = id * tamano_chunk;
        int limite = (id+1) * tamano_chunk ;
        int ultimo_primo=indice;
        while(ultimo_primo <= dimension && !numeros_marcados[ultimo_primo]) ++ultimo_primo; 

        for ( indice = ultimo_primo ;  ultimo_primo < limite  && indice <= dimension; ++indice) 
        {
            if(numeros_marcados[indice])
            {
                maximo_gap = max(maximo_gap ,indice-ultimo_primo);
                ultimo_primo = indice;
            }
        }      
    }

    t_fin = omp_get_wtime();
    printf("%i ,%16g \n",numero_hilos,t_fin-t_ini);

	cout<<numero_hilos<<" ,"<<(t_fin - t_ini)<<"\n";
    // Imprimir la informacion
    //cout<<"Criba hasta el numero: "<<dimension<<'\n';
    //cout<<"Mayor gap: "<<maximo_gap<<'\n';
    //Imprimir_Primos(numeros_marcados,dimension);
        
    delete [] numeros_marcados;
    return 0;
}
