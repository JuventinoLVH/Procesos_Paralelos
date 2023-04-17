#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "Cronometro.h"

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
    int dimension = atoi(argv[1]);
    bool *numeros_marcados;
    numeros_marcados = new bool[dimension+1];
    if (numeros_marcados == NULL) {
        cout<<"Cannot allocate enough memory\n";
        exit (1);
    }


    // Marcamos a todos los numeros, asumimos que todos los numeros son primos. 
    for(int i = 0 ; i <= dimension ; i++ ){
        numeros_marcados[i] = true;
    }
    numeros_marcados[0] = false;
    numeros_marcados[1] = false;


    // Empezamos a marcar desde el 2, el primer primo.
    int index = 2,ultimo_primo = 2;
    do
    {
        for (int i = index + ultimo_primo ; i <= dimension; i += ultimo_primo)
            numeros_marcados[i] = false; // marcando a los multiplos de index

        while(!numeros_marcados[++index] && index < dimension); // buscando el siguiente valor sin marcar
        ultimo_primo = index;
    }while (index < dimension);


    //Contamos el numero de primos 
    int total_primos = 0;
    for (int i = 2; i <= dimension; i++)
    {
        if (numeros_marcados[i])
            ++total_primos;
    }


    //Calculamos el maximo gap, se podria juntar con el ciclo anterior, 
    // pero no lo hago para practicar con la paralelisacion.
    ultimo_primo=2;
    int maximo_gap=0;
    for (int i = 3; i <= dimension; i++)
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
    cout<<"Criba hasta el numero: "<<dimension<<'\n';
    cout<<"cantidad de numeros primos: "<<total_primos<<'\n';
    cout<<"Mayor gap: "<<maximo_gap<<'\n';
    Imprimir_Primos(numeros_marcados,dimension);
    
    system("pause");    
    delete [] numeros_marcados;
    return 0;
}
