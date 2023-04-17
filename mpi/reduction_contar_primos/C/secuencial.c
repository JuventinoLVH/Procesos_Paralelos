#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define _BLOCK_LOW(id, n_hilos, dimencion) ((id) * (dimencion) / (n_hilos))
#define _BLOCK_HIGH(id, n_hilos, dimencion) (_BLOCK_LOW((id) + 1, (n_hilos), (dimencion)) - 1)


int Es_primo(int numero);

int main (int argc, char *argv[] ) 
{
    int dimencion = atoi(argv[1]);    
    int resultado = 0;
    int indice = 0;
    

    double time_spent = 0.0;
    clock_t begin = clock();
    // calcula el tiempo transcurrido encontrando la diferencia (end - begin) y
    // dividiendo la diferencia por CLOCKS_PER_SEC para convertir a segundos
    
    for(indice = 2; indice <= dimencion ; ++indice)
    {
        resultado += Es_primo(indice);
    }

    
    clock_t end = clock();
    time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
    printf("%i ,%f ,%i \n", 1 ,time_spent, resultado);

    
    return 0;
}



inline int Es_primo(int numero)
{
    if(numero < 2) return 1;

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
