#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
// -------------------------------
float time_diff(struct timeval *start, struct timeval *end){
    return (end->tv_sec - start->tv_sec) + 1e-6*(end->tv_usec - start->tv_usec);
}
// -------------------------------
int es_primo(int m){
    int i,ind;
    ind = 1;
    for( i = 2 ; i <= sqrt(m) && ind == 1 ; i++ )
        if( m % i == 0 ){
            ind = 0;
            break;
        }
    return ind;
}
// -------------------------------
int main (int argc, char** argv){
    int n, c, num_primos=0;
    struct timeval start;
    struct timeval end;    

    if(argc!=2){
        printf("se requiere que se envien 2 argumentos");        
        return 1;
    }

    gettimeofday(&start, NULL);// <----- tiempo1

    n = atoi(argv[1]);

    for( c = 2 ; c <= n ; c++ )
        num_primos += es_primo(c);
    
    gettimeofday(&end, NULL); // <----- tiempo2

    //printf("Total de numeros primos: %d en el rango de [1, %d], %f segundos\n", num_primos, n, time_diff(&start, &end));
    printf("%f",time_diff(&start, &end) );
    
    return 0;
}
