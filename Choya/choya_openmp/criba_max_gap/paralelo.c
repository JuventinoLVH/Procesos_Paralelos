/*
  Criba de Eratóthenes
  programado por : José Alberto Leyva
  15 de octubre de 2021
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#define BLOCK_LOW  
#define BLOCK_HIG
#define BLOCK_SIZE


void Imprimir_Primos(int marked[], int &n){
    
    printf("PRIMOS = [");
    for (i = 0; i < n; i++) 
        if (!marked[i]) 
            printf("%d,",i+2);
    printf("]");
}



int main(int argc, char *argv[]){
    int n, k, i, count, index = 0;
    char  *marked;
  
    struct timeval inicio, fin;
    gettimeofday(&inicio,0); // toma de tiempo inicial

    if (argc != 2) { printf ("Command line: %s <m>\n", argv[0]); exit (1);}

    n = atoi(argv[1]);
    marked = (char *) malloc (n); // para quitar el 1
    if (marked == NULL) {
        printf ("Cannot allocate enough memory\n");
        exit (1);
    }

    
    //#pragma omp parallel for private(i) firstprivate(n) shared(marked) <- Primer intento
    #pragma omp parallel for
    for( i = 0 ; i < n ; i++ ){      
      marked[i] = 0;
    } 

    

    k = 2; // primer primo   
    do {
        for (i = k*k - 2; i < n-1; i += k)
            marked[i] = 1; // marcando a los múltiplos de k
       
        while (marked[++index]); // buscando el siguiente valor sin marcar
        k = index + 2;       
    }while (k  k <= n); 
    

    // contando el número de valores sin marcar (el número de ceros en el arreglo marked)
    count = 0;
    /* De forma secuencial
        for (i = 0; i < n-1; i++) 
            if (!marked[i]) 
                count++;
    */
    
    #pragma omp parallel for private(i) firstprivate(n) reduction(+: count)
    for(i=0;i<n-1;i++)        
        if(!marked[i])
            count++;

    
    // toma de tiempo final
    gettimeofday(&fin,0); 

    Imprimir_Primos(marked,n)

    long seconds = fin.tv_sec - inicio.tv_sec;
    long microseconds = fin.tv_usec - inicio.tv_usec;
    double diff_t = seconds + microseconds*1e-6;  

   // printf ("Hay %d números primos menores o iguales que %d\n", count, n);
    printf("\n %.10f \n",diff_t);  
 
    return 0;
}