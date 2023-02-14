#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "Cronometro.h"
#include <omp.h>

#define BLOCK_LOW  
#define BLOCK_HIG
#define BLOCK_SIZE

using namespace std;

void Imprimir_Primos(bool numeros_marcados[], int &n){

    cout<<"PRIMOS = [";
    for (int i = 2; i <= n; i++)
        if (numeros_marcados[i])
            cout<<i<<", ";
    cout<<"\b\b]\n";
}


int main(int argc, char *argv[])
{

    // toma de tiempo inicial
    Cronometro timer;
    timer.Iniciar();

    // Se crea un arreglo
    int dimencion = atoi(argv[1]);
    bool *numeros_marcados;
    numeros_marcados = new bool[dimencion+1];
    if (numeros_marcados == NULL) {
        cout<<"Cannot allocate enough memory\n";
        exit (1);
    }


    // Marcamos a todos los numeros, asumimos que todos los numeros son primos.
    #pragma omp parallel for private(i) firstprivate(dimencion) shared(numeros_marcados)
    for(int i = 0 ; i <= dimencion ; i++ ){
        numeros_marcados[i] = true;
    }
    numeros_marcados[0] = false;
    numeros_marcados[1] = false;


    // Empezamos a marcar desde el 2, el primer primo.
    //#pragma omp parallel for private(i) firstprivate(n) reduction(+: count)
    int index = 2,ultimo_primo = 2;
    do
    {
        for (int i = index + ultimo_primo ; i <= dimencion; i += ultimo_primo)
            numeros_marcados[i] = false; // marcando a los multiplos de index

        while(!numeros_marcados[++index] && index < dimencion); // buscando el siguiente valor sin marcar
        ultimo_primo = index;
    }while (index < dimencion);


    //Contamos el numero de primos
    #pragma omp parallel for private(i) firstprivate(dimencion) reduction(+: total_primos)
    int total_primos = 0;
    for (int i = 2; i <= dimencion; i++)
    {
        if (numeros_marcados[i])
            ++total_primos;
    }


    //Calculamos el maximo gap, se podria juntar con el ciclo anterior, 
    // pero no lo hago para practicar con la paralelisacion.
    ultimo_primo=2;
    int maximo_gap=0;
    #pragma omp parallel for private(i) firstprivate(dimencion) reduction(+: total_primos)
    for (int i = 3; i <= dimencion; i++)
    {     
        if (numeros_marcados[i])
        {
            maximo_gap = max(maximo_gap ,
             i-ultimo_primo);
            ultimo_primo = i;
        }
    }
    

    // Imprimir la informacion
    cout<<"Tiempo tomado: "<<fixed << timer.get_tiempo()<<'\n';
    cout<<"Criba hasta el numero: "<<dimencion<<'\n';
    cout<<"cantidad de numeros primos: "<<total_primos<<'\n';
    cout<<"Mayor gap: "<<maximo_gap<<'\n';
    Imprimir_Primos(numeros_marcados,dimencion);
    
    system("pause");    
    delete [] numeros_marcados;
    return 0;
}
