#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char Comprobar(int *arreglo, int tamano)
{
    int i;
    for(i=1 ; i < tamano ; ++i)
    {
        if(arreglo[i-1] > arreglo[i])
        {
            return 'E';
        }
    }
    return 's';
}

int read_array(char* fname, int **arr) {
    FILE *myFile;
    int i,n;

    myFile = fopen(fname, "r");
    fscanf(myFile, "%i\n", &n); // <--- primero lee el num de datos a ordenar

    *arr = (int *) malloc(n*sizeof(int));
    if (*arr == NULL){printf("Memoria insuficiente\n");return 1;}

    for (i=0; i < n; i++) fscanf(myFile, "%i\n", (*arr)+i);

    return n;
}

int funcionQueCompara(const void *a, const void *b) {
    // Castear a enteros
    int aInt = *(int *) a;
    int bInt = *(int *) b;
    // Al restarlos, se debe obtener un número mayor, menor o igual a 0
    // Con esto ordenamos de manera ascendente
    return aInt - bInt;
}

int main(int argc, char*argv[]) {

    int *numbers;
    int n, i;
    float seconds;    
    n = read_array(argv[1], &numbers);

    
    clock_t start = clock();
    
    int rep;
    qsort(numbers, n, sizeof(int), funcionQueCompara);
    
    seconds = (float)(clock() - start) / CLOCKS_PER_SEC;


    printf("1, %.10f ,%c\n", seconds, Comprobar(numbers,n));
    return 0;
}