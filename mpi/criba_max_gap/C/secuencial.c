#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define _BLOCK_LOW(id, n_hilos, dimencion) ((id) * (dimencion) / (n_hilos))
#define _BLOCK_HIGH(id, n_hilos, dimencion) (_BLOCK_LOW((id) + 1, (n_hilos), (dimencion)) - 1)
#define _MAX(x,y)( x > y ? x : y)


int Es_primo(int numero);

int main (int argc, char *argv[] ) 
{
    int dimencion = atoi(argv[1]);    
    int resultado = 0;
    int indice = 0;
    

    double time_spent = 0.0;
    clock_t begin = clock();
    int primo_anterior=2;
    int max_gap=0;
 
 
    // calcula el tiempo transcurrido encontrando la diferencia (end - begin) y
    // dividiendo la diferencia por CLOCKS_PER_SEC para convertir a segundos                
    for(indice = 3; indice <= dimencion ; ++indice)
    {
        if(Es_primo(indice))
        {
            max_gap = _MAX(max_gap,(indice - primo_anterior));
            primo_anterior = indice;
        }
    }

    
    clock_t end = clock();
    time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
    printf("%i ,%f ,%i \n", 1 ,time_spent, max_gap);

    
    return 0;
}



int Es_primo(int numero)
{
    if(numero < 2) return 0;

    int i=2;
	for (i; i * i  <= numero ; i++)
    {
        if (numero % i  == 0) 
        {
            return 0;
        } 
    };
    return 1;
}
