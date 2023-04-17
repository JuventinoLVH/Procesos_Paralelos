#include <iostream>
#include <sys/time.h>
#include "Cronometro.h"
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
    

    // Se crea un arreglo
    int dimension = atoi(argv[1]);
    bool *numeros_marcados;
    numeros_marcados = new bool[dimension+1];
    if (numeros_marcados == NULL) {
        cout<<"Cannot allocate enough memory\n";
        exit (1);
    }
    numeros_marcados[0] = numeros_marcados[1] = false;
    int numero_hilos = atoi(argv[2]);

    // toma de tiempo inicial
    Cronometro timer;
    timer.Iniciar();
    
    #pragma omp parallel num_threads(numero_hilos) \
        shared(numeros_marcados,dimension,)
    {
        // Marcamos a todos los numeros, asumimos que todos los numeros son primos.
        for(int i = 2 ; i <= dimension ; i++ ){
            numeros_marcados[i] = true;
        }
    }


    // Empezamos a marcar desde el 2, el primer primo.    
    int indice = 2,ultimo_primo = 2;
    do
    {
        // marcando a los multiplos del numero primo
        #pragma omp parallel for num_threads(numero_hilos) \
            shared(indice,ultimo_primo,numeros_marcados)
        {
            for (int i = indice + ultimo_primo ; i <= dimension; i += ultimo_primo)
                numeros_marcados[i] = false;
        }
        //Nota, aqui hay una barrera implicita, el hilo en single espera 
        //a que se terimne la ejecucion del ciclo 

        // buscando el siguiente valor sin marcar
        while(!numeros_marcados[++indice] && indice < dimension); 
        ultimo_primo = indice;
    }while (indice < dimension);

        
     //Contamos el numero de primos y el maximo gap
    int maximo_gap=1;
    int total_primos = 0;
    int tamano_chunk = (dimension / numero_hilos)+1;

    #pragma omp parallel num_threads(numero_hilos) \
        shared(total_primos,numeros_marcados,dimension,tamano_chunk) \
        reduction(max : maximo_gap)
    {
        int id = omp_get_thread_num();
        int indice = id * tamano_chunk;
        int limite = (id+1) * tamano_chunk ;
        int ultimo_primo=indice;
        while(!numeros_marcados[ultimo_primo] && ultimo_primo <= dimension) ++ultimo_primo; 

        for ( indice = ultimo_primo ;  ultimo_primo < limite ; ++indice) 
        {
            if(numeros_marcados[ultimo_primo])
            {
                maximo_gap = max(maximo_gap ,indice-ultimo_primo);
                ultimo_primo = indice;
// 
                #pragma omp atomic
                ++total_primos;
            }
        }      
    }

    // Imprimir la informacion
    cout<<"Tiempo tomado: "<<fixed << timer.get_tiempo()<<'\n';
    cout<<"Criba hasta el numero: "<<dimension<<'\n';
    cout<<"cantidad de numeros primos: "<<total_primos<<'\n';
    cout<<"Mayor gap: "<<maximo_gap<<'\n';
    Imprimir_Primos(numeros_marcados,dimension);
    
    system("pause");    
    delete [] numeros_marcados;
    return 0;
}
