#include <stdlib.h>
#include "mpi.h"
#include <stdio.h>
#include <math.h> // ceil()
#include <limits.h> // INT_MAX 
// 
int funcionQueCompara(const void *a, const void *b) {
    // Castear a enteros
    int aInt = *(int *) a;
    int bInt = *(int *) b;
    // Al restarlos, se debe obtener un número mayor, menor o igual a 0
    // Con esto ordenamos de manera ascendente
    return aInt - bInt;
}
void mezcla(int *S_i, int *aux, int chunksize){
    int i = 0, j = 0, k = 0, z; 

    // copiar en array temporal
    for (z = 0; z < 2*chunksize; z++) 
        aux[z] = S_i[z];
    
    // merge
    while (i < chunksize && j < chunksize) {
        if (aux[i] <= aux[j+chunksize]) {
            S_i[k] = aux[i];
            i++;
        } else {
            S_i[k] = aux[j+chunksize];
            j++;
        }
        k++;
    }

    while (i < chunksize) {
        S_i[k] = aux[i];
        i++;
        k++;
    }

    while (j < chunksize) {
        S_i[k] = aux[j+chunksize];
        j++;
        k++;
    }

    return ;
}
//  ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
int read_array(char* fname, int **arr, int np) {
  FILE *myFile;
  unsigned int i, n, chunksize, faltantes;

  myFile = fopen(fname, "r");
  if(!myFile){
    printf("ERROR: No se pudo abrir el archivo para lectura %s",fname);
    MPI_Abort(MPI_COMM_WORLD, 99);
  }

  fscanf(myFile, "%i\n", &n); // numero de datoss a leer

  chunksize = ceil(n*1.0 / np);
  faltantes = np*chunksize - n;

  *arr = (int *) malloc((chunksize*np)*sizeof(int));
  if (*arr == NULL) {    printf("Memoria insuficiente\n");    MPI_Abort(MPI_COMM_WORLD, 99);  }

  for ( i=0; i < n; i++)
    fscanf(myFile, "%i\n", (*arr)+i);

  for ( i=n; i < n + faltantes; i++) //<--- Dummys
    (*arr)[i] = INT_MAX;

  return n;
}

//  ■■■■■■■■■■■■■■■■■■■■■■■■■■
int main(int argc, char**argv) {
  int rank, np, root, faltantes;
  unsigned int n, chunksize, i;
  double start, tiempo;
  int *Arreglo, *S_i, *auxiliar;

  MPI_Init(&argc, &argv);

  MPI_Comm_size(MPI_COMM_WORLD, &np); // Numero total de procesos
  MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Valor de nuestro identificador

  MPI_Barrier(MPI_COMM_WORLD);  // sincronización
  start = MPI_Wtime();

  // Root es el último
  root = np - 1;

  // <--- toma de tiempo 1
  if (rank == root){ // Lee los datos del archivo
    n = read_array(argv[1], &Arreglo, np);
    chunksize = ceil(n*1.0 / np);
    faltantes = np*chunksize - n;    

    //printf("\nArreglo = {");
    //for(i=0;i<n;i++)
      //printf("%d, ", Arreglo[i]);
    //printf("}");

  }
  
  // Comunica longitud de los chunks
  MPI_Bcast(&chunksize, 1, MPI_UNSIGNED, root, MPI_COMM_WORLD);
  
  // Pide memoria del doble de dicha longitud para poder ordenar
  S_i = (int *) malloc(2*chunksize*sizeof(int));  if (S_i == NULL) {   printf("Memoria insuficiente\n");    MPI_Abort(MPI_COMM_WORLD, 99);  }
  auxiliar = (int *) malloc(2*chunksize*sizeof(int));  if (auxiliar == NULL) {   printf("Memoria insuficiente (auxiliar)\n"); MPI_Abort(MPI_COMM_WORLD, 98); }

  MPI_Scatter(Arreglo, chunksize, MPI_INT, S_i, chunksize, MPI_INT, root, MPI_COMM_WORLD);

  if (rank==0) {
    //for(i=0;i<chunksize;i++)
      //printf("%d\t ", S_i[i]);
  }

  // ------aqui va el procesamiento a realizar con los datos

  qsort(  S_i, chunksize, sizeof(int), &funcionQueCompara ); // ordenamiento local

  for(i=1;i<=ceil(np/2.0);i++){
    if(rank%2==0 && rank!=np-1){ // los pares hacen la mezcla
      MPI_Recv( S_i+chunksize, chunksize, MPI_INT, rank+1, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE );   
      mezcla(S_i,auxiliar, chunksize); // <----  
      MPI_Send(S_i+chunksize, chunksize, MPI_INT, rank+1, 10, MPI_COMM_WORLD);  
    }

    if(rank%2==1) {
      MPI_Send( S_i, chunksize, MPI_INT, rank-1, 10, MPI_COMM_WORLD );
      MPI_Recv( S_i, chunksize, MPI_INT, rank-1, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
    }


    if(rank%2==1 && rank!=np-1){ // los impares hacen la mezcla
      MPI_Recv( S_i+chunksize, chunksize, MPI_INT, rank+1, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE );   
      mezcla(S_i,auxiliar, chunksize); // <---- 
      MPI_Send(S_i+chunksize, chunksize, MPI_INT, rank+1, 10, MPI_COMM_WORLD);  
    }

    if(rank%2==0 && rank!=0) {
      MPI_Send( S_i, chunksize, MPI_INT, rank-1, 10, MPI_COMM_WORLD );
      MPI_Recv( S_i, chunksize, MPI_INT, rank-1, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
    }

  }
  // ---------------------------------------------------------

  // Recolectar de nuevo los datos en el root e imprimir
  MPI_Gather(S_i, chunksize, MPI_INT, Arreglo, chunksize, MPI_INT, root, MPI_COMM_WORLD );
    
  tiempo = MPI_Wtime() - start;  // <--- toma de tiempo 2

  int ordendado = 1;
  if( rank==root ){
    /*
    printf("\nArreglo ordenado = {");
    for(i=0;i<n;i++)
      printf("%d\t",Arreglo[i]);
    printf("}");
    */

    for( i=1; i<n; i++ ){
        if(Arreglo[i-1]>Arreglo[i]){
            ordendado=0;
            break;
        }
    }
  }

  //printf("\%d, %.10f, %d  \n",numThreads, elapsed_time, ordendado); 

  if(rank==root)
    printf("\%d, %.10f, %d  \n", np, tiempo, ordendado);

  // ---------------------------------------------------------
  free(S_i);
  if (rank == root) free(Arreglo);

  MPI_Finalize();
  return 0;
}
