#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#define BLOCK_LOW
#define BLOCK_HIG
#define BLOCK_SIZE

using namespace std;
void Imprimir_Primos(bool marked[], int &n){

    cout<<"PRIMOS = [";
    for (int i = 2; i <= n; i++)
        if (marked[i])
            cout<<i<<", ";
    cout<<"\b\b]";
}


int main(int argc, char *argv[])
{
    // toma de tiempo inicial
    int dimension;
    cin>>dimension;

    timeval inicio, fin;
    gettimeofday(&inicio,0);


    bool *marked;
    marked = new bool[dimension+1]; // para quitar el 1
    if (marked == NULL) {
        printf ("Cannot allocate enough memory\n");
        exit (1);
    }


    //#pragma omp parallel for private(i) firstprivate(n) shared(marked) <- Primer intento
    //#pragma omp parallel for
    for(int i = 0 ; i < dimension ; i++ ){
        marked[i] = true;
    }



    int index = 2,ultimo_primo = 2;
    do
    {
        for (int i = index + index ; i <= dimension; i += ultimo_primo)
            marked[i] = false; // marcando a los múltiplos de k

        while(!marked[++index]); // buscando el siguiente valor sin marcar
        ultimo_primo = index;
    }while (ultimo_primo < dimension);

    int total_primos = 0;
    for (int i = 2; i <= dimension; i++)
    {
        if (marked[i])
            ++total_primos;
    }


    /*
    #pragma omp parallel for private(i) firstprivate(n) reduction(+: count)
    for(i=0;i<n-1;i++)
        if(!marked[i])
            count++;
    */

    // toma de tiempo final
    gettimeofday(&fin,0);



    long seconds = fin.tv_sec - inicio.tv_sec;
    long microseconds = fin.tv_usec - inicio.tv_usec;
    double diff_t = seconds + microseconds*1e-6;

    Imprimir_Primos(marked,dimension);
    printf("\n %.10f \n",diff_t);

    delete [] marked;
    return 0;
}
