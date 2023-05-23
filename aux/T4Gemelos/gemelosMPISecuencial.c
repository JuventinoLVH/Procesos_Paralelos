#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int main(int argc, char *argv[]){
    int n, k, i, count, index = 0, temp;
    char  *marked;

    struct timeval inicio, fin;


    if (argc != 2) { printf ("Command line: %s <m>\n", argv[0]); exit (1);}

    n = atoi(argv[1]);
    marked = (char *) malloc (n-1); // para quitar el 1
    if (marked == NULL) {printf ("Cannot allocate enough memory\n");exit (1);}

    gettimeofday(&inicio,0); // toma de tiempo inicial
    for (i = 0; i < n-1; i++)
        marked[i] = 0;

    k = 2; // primer primo
    do {
        for (i = k*k - 2; i < n-1; i += k)
            marked[i] = 1; // marcando a los múltiplos de k

        while (marked[++index]); // buscando el siguiente valor sin marcar
        k = index + 2;
    }while (k * k <= n);

    count = 0;
    for(i = 1; i < n-1; i++){
      if(!marked[i] && !marked[i-2]){;
        count++;
        //printf("i:%d, Count: %d \n", i, count);
      }
    }
    if(!marked[n-1]) count--;

    /*
    for (i = 0; i < n-1; i++)
        if (!marked[i])
            count++;
    */

    gettimeofday(&fin,0); // toma de tiempo final
    /*
    printf("PRIMOS = [");
    for (i = 0; i < n-1; i++)
        if (!marked[i])
            printf("%d,",i+2);
    printf("]");
    */

    long seconds = fin.tv_sec - inicio.tv_sec;
    long microseconds = fin.tv_usec - inicio.tv_usec;
    double diff_t = seconds + microseconds*1e-6;

    printf ("Hay %d parejas de números primos gemelos menores o iguales que %d\n", count, n);
    printf("\nTiempo =  %.10f, Gem: %d \n",diff_t, count);

    return 0;
}


