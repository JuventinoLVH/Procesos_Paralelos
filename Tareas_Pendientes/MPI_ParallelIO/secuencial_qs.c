#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//  ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
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

//  ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
int funcionQueCompara(const void *a, const void *b) {
    // Castear a enteros
    int aInt = *(int *) a;
    int bInt = *(int *) b;
    // Al restarlos, se debe obtener un número mayor, menor o igual a 0
    // Con esto ordenamos de manera ascendente
    return aInt - bInt;
}

//   ■■■■■■■■■■■■■■■■■■■■■■■■■■■
void main(int argc, char*argv[]) {
    int *numbers;
    int n, i;
    float seconds;
  
    clock_t start = clock();
    n = read_array(argv[1], &numbers);

    /*printf("\n Núm de datos = %d",n);
    printf("\n Desordenados-->");
    for( i=0; i<n; i++ )
        printf("%d, ",numbers[i]);*/

   
    qsort(numbers, n, sizeof(int), funcionQueCompara);
    seconds = (float)(clock() - start) / CLOCKS_PER_SEC;

    /* Comprobación de ordenamiento bien realizado */
    for (i = 0; i < n-1; i ++){
        if (numbers[i] > numbers[i+1]){
            printf("ERROR: Hay un par mal ordenado !!!!!");
            return;
        }
    }  

    /*printf("\n Ordenados-->\t");
    for( i=0; i<n; i++ )
        printf("%d, ",numbers[i]);*/
  
    printf("1, %.10f, 0 \n", seconds);
    return;
}
