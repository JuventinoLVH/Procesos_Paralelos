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
void mezcla(int *S_i, int chunksize){
    int i = 0, j = 0, k = 0, z;
    int *aux; 
    aux = (int *) malloc(2*chunksize*sizeof(int));  if (aux == NULL) {   printf("Memoria insuficiente (auxiliar)\n"); MPI_Abort(MPI_COMM_WORLD, 98); }

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
    free(aux);

    return ;
}
//  ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

#define BLOCK_LOW(id,p,n)  ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id)+1,p,n)-1)
#define BLOCK_SIZE(id,p,n) (BLOCK_HIGH(id,p,n)-BLOCK_LOW(id,p,n)+1)
#define BLOCK_OWNER(j,p,n) (((p)*((j)+1)-1)/(n))
//  ■■■■■■■■■■■■■■■■■■■■■■■■■■
int main(int argc, char**argv) {
  int rank, np, root, faltantes;
  unsigned int n, chunksize, i;
  double start, tiempo;
  int *Arreglo, *arreglo_local;
  MPI_Offset  *offset;
  MPI_Status  status;
  MPI_File fh;

  MPI_Init(&argc, &argv);

  MPI_Comm_size(MPI_COMM_WORLD, &np); // Numero total de procesos
  MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Valor de nuestro identificador

  MPI_Barrier(MPI_COMM_WORLD);  // sincronización
  start = MPI_Wtime();

  // Root es el último
  root = np - 1;

  offset = (MPI_Offset*)malloc(np*sizeof(MPI_Offset));if(offset==NULL){ printf("\n ERROR: No hay memoria suficiente (offset)"); MPI_Abort(MPI_COMM_WORLD, 99); }
  MPI_File_open( MPI_COMM_WORLD, argv[1], MPI_MODE_RDONLY, MPI_INFO_NULL  , &fh );
      MPI_File_read_all(fh, &n, 1, MPI_INT, &status);

      offset[0] = sizeof(int); // <--- se salta el valor n 
      for( i = 1 ; i < np ; i++ ) 
          offset[i] = offset[i-1] + BLOCK_SIZE( i-1, np, n )*sizeof(int);

      chunksize = ceil(n*1.0 / np);
      faltantes = np*chunksize - n;

      arreglo_local = (int*)malloc(2*chunksize*sizeof(int)); if( arreglo_local == NULL ){ printf("\n ERROR: No hay memoria suficiente (buffer)"); MPI_Abort(MPI_COMM_WORLD, 99); }
      
      MPI_File_read_at_all(fh, offset[rank], arreglo_local, BLOCK_SIZE( rank, np, n ), MPI_INT, &status); 

      if( BLOCK_SIZE( rank, np, n )!=chunksize  ){
        (arreglo_local)[chunksize-1] = INT_MAX;
      }

  MPI_File_close(&fh);   

  if(rank == root){
      Arreglo = (int*)malloc(chunksize*np*sizeof(int)); if( Arreglo == NULL ){ printf("\n ERROR: No hay memoria suficiente (Arreglo)"); MPI_Abort(MPI_COMM_WORLD, 99); }
  }
  MPI_Barrier(MPI_COMM_WORLD);

  // ------aqui va el procesamiento a realizar con los datos

  qsort(  arreglo_local, chunksize, sizeof(int), &funcionQueCompara ); // ordenamiento local

  for(i=1;i<=ceil(np/2.0);i++){
    if(rank%2==0 && rank!=np-1){ // los pares hacen la mezcla
      MPI_Recv( arreglo_local+chunksize, chunksize, MPI_INT, rank+1, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE );   
      mezcla(arreglo_local, chunksize); // <----  
      MPI_Send(arreglo_local+chunksize, chunksize, MPI_INT, rank+1, 10, MPI_COMM_WORLD);  
    }

    if(rank%2==1) {
      MPI_Send( arreglo_local, chunksize, MPI_INT, rank-1, 10, MPI_COMM_WORLD );
      MPI_Recv( arreglo_local, chunksize, MPI_INT, rank-1, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
    }


    if(rank%2==1 && rank!=np-1){ // los impares hacen la mezcla
      MPI_Recv( arreglo_local+chunksize, chunksize, MPI_INT, rank+1, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE );   
      mezcla(arreglo_local, chunksize); // <---- 
      MPI_Send(arreglo_local+chunksize, chunksize, MPI_INT, rank+1, 10, MPI_COMM_WORLD);  
    }

    if(rank%2==0 && rank!=0) {
      MPI_Send( arreglo_local, chunksize, MPI_INT, rank-1, 10, MPI_COMM_WORLD );
      MPI_Recv( arreglo_local, chunksize, MPI_INT, rank-1, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
    }

  }
  // ---------------------------------------------------------

  // Recolectar de nuevo los datos en el root e imprimir
  MPI_Gather(arreglo_local, chunksize, MPI_INT, Arreglo, chunksize, MPI_INT, root, MPI_COMM_WORLD );

  tiempo = MPI_Wtime() - start;  // <--- toma de tiempo 2

  int ordendado = 1;
  if( rank==root ){
    // printf("\nArreglo ordenado = {");
    // for(i=0;i<n;i++){
    //   printf("%d\t",Arreglo[i]);
    //   count++;
    // }
    //printf("}\n");
    
    for( i=1; i<n; i++ ){
        if(Arreglo[i-1]>Arreglo[i]){
            ordendado=0;
            break;
        }
    }
  }
  
  if(rank==root)
    printf("\%d, %.10f, %d  \n", np, tiempo, ordendado);

  // ---------------------------------------------------------
  free(arreglo_local);
  free(offset);
  if (rank == root) free(Arreglo);
  MPI_Finalize();
  return 0;
}
