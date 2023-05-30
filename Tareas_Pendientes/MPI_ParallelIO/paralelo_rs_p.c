#include <stdlib.h>
#include "mpi.h"
#include <stdio.h>
#include <math.h> // ceil()
#include <limits.h> // INT_MAX 

#define BLOCK_LOW(id,p,n) ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id)+1,p,n)-1)
#define BLOCK_SIZE(id,p,n)(BLOCK_HIGH(id,p,n)-BLOCK_LOW(id,p,n)+1)
//  ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
int compare(const void *_a, const void *_b) { 
    int *a, *b;
    
    a = (int *) _a;
    b = (int *) _b;
    return (*a - *b);
}

//  ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
void merge(int *A, int *numA, int *B, int numB) {
    int i = 0, j = 0, k = 0;
    int numAux = (*numA) + numB;
    int *aux = (int*) malloc( numAux * sizeof(int) ); if (aux == NULL) {   printf("Memoria insuficiente (auxiliar)\n"); MPI_Abort(MPI_COMM_WORLD, 98); }
    
    while (j < (*numA) && k < numB){ aux[i++] = (A[j] < B[k]) ? A[j++] : B[k++]; }
    
    while (j < (*numA)){ aux[i++] = A[j++]; }
    
    while (k < numB){ aux[i++] = B[k++]; }
    
    for (i = 0; i < numAux; i++){ A[i] = aux[i]; }
    
    (*numA) = numAux;
    free(aux);
}

//  ■■■■■■■■■■■■■■■■■■■■■■■■■■
int main(int argc, char**argv) {
    int id, np, root;
    unsigned int n, i,j, n_local, *displs, *sendcounts,*recvcounts,*sdispls,*rdispls, ndatos_recv;
    double start, time;
    int *sendbuf, *arreglo, *muestras, *recvbuf_mezclas, *arreglo_local;
    int *todas_las_muestras, *pivotes;
    MPI_Offset  *offset;
    MPI_Status  status;
    MPI_File fh,fh2;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &np); // Numero total de procesos
    MPI_Comm_rank(MPI_COMM_WORLD, &id); // Valor de nuestro identificador

    MPI_Barrier(MPI_COMM_WORLD);  // sincronización
    start = MPI_Wtime();

    // El root lee un dataset del archivo con n datos e inicia una dispersión de los datos.  <-----  (1)
    root = np - 1;

    offset = (MPI_Offset*)malloc(np*sizeof(MPI_Offset));if(offset==NULL){ printf("\n ERROR: No hay memoria suficiente (offset)"); MPI_Abort(MPI_COMM_WORLD, 99); }
    MPI_File_open( MPI_COMM_WORLD, argv[1], MPI_MODE_RDONLY, MPI_INFO_NULL  , &fh );
        MPI_File_read_all(fh, &n, 1, MPI_INT, &status);

        offset[0] = sizeof(int); // <--- se salta el valor n 
        for( i = 1 ; i < np ; i++ ) 
            offset[i] = offset[i-1] + BLOCK_SIZE( i-1, np, n )*sizeof(int);

        n_local = BLOCK_SIZE( id, np, n );

        arreglo_local = (int*)malloc(n_local*sizeof(int)); if( arreglo_local == NULL ){ printf("\n ERROR: No hay memoria suficiente (buffer)"); MPI_Abort(MPI_COMM_WORLD, 99); }
        
        MPI_File_read_at_all(fh, offset[id], arreglo_local, n_local, MPI_INT, &status); 

     MPI_File_close(&fh);   

    sendcounts = (int*)malloc(np*sizeof(int));
    if (sendcounts == NULL) {printf("Memoria insuficiente (sendcounts) \n");MPI_Abort(MPI_COMM_WORLD, 99);}

    if(id==root){
    
        for(i=0;i<np;i++)
            sendcounts[i] = BLOCK_SIZE(i,np,n); 
        
        //printf("\n[%d] sendcounts={",id);  for(i=0;i<np;i++) printf("%d, ", sendcounts[i]); printf("}\n"); // <---- impresion

        displs = (int*)malloc(np*sizeof(int)); 
        if(displs==NULL){ printf("ERROR: Memoria insuficiente (displs)"); MPI_Abort(MPI_COMM_WORLD,99); } 
        displs[0]=0; 
        for(i=1;i<np;i++)
            displs[i] = displs[i-1]+sendcounts[i-1];

        
        arreglo = (int*)malloc(n*sizeof(int)); if( arreglo == NULL ){ printf("\n ERROR: No hay memoria suficiente (Arreglo)"); MPI_Abort(MPI_COMM_WORLD, 99); }
  
    }
    

    
    // Cada proceso ordena localmente sus datos con QuickSort  <-----  (2)
    qsort(arreglo_local, n_local, sizeof(int), &compare);


    // Cada proceso selecciona los n índices 0, n/p^2, 2n/p^2, 3n/p^2,..., (p-1)n/p^2 como muestras regulares <-----  (3)
    muestras = (int*)malloc(np*sizeof(int));
    if (muestras == NULL) {printf("Memoria insuficiente (muestras)\n");MPI_Abort(MPI_COMM_WORLD, 99);}

    for(i=0;i<np;i++)
        muestras[i] = arreglo_local[ i*n/(np*np) ];


    // Un proceso recolecta y ordena las muestras regulares y selecciona p-1 valores pivote de la lista ordenada. <-----  (4)
    // Los valores pivote están en los índices p+p/2-1, 2p+p/2-1, 3p+p/2-1, ... , (p-1)p+p/2-1.   
    // Los valores pivote son dados a conocer a todos los procesos.

    if(id==root){
        todas_las_muestras = (int*)malloc(   np*np*sizeof(int) );
        if (todas_las_muestras == NULL) {printf("Memoria insuficiente (todas_las_muestras)\n");MPI_Abort(MPI_COMM_WORLD, 99);}
    }

    MPI_Gather( muestras, np, MPI_INT, todas_las_muestras, np, MPI_INT, root, MPI_COMM_WORLD );

    
    if(id==root){
        qsort( todas_las_muestras, np*np, sizeof(int), &compare );

    }
    pivotes = (int*)malloc( (np-1)*sizeof(int)   );
    if (pivotes == NULL) {printf("Memoria insuficiente (pivotes) \n");MPI_Abort(MPI_COMM_WORLD, 99);}

    if(id==root){
        for(i=0;i<np-1;i++)
        pivotes[i] = todas_las_muestras[ (i+1)*np + np/2 -1 ];    
    }


    MPI_Bcast(pivotes, np-1, MPI_INT, root, MPI_COMM_WORLD);
    
    // Cada proceso particiona su sublista ordenada en p piezas disjuntas, usando los valores pivote como separadores. <----- (5)
    j=0; 
    for(i=0;i<(np-1);i++){
        sendcounts[i]=0;
        while(arreglo_local[j]<=pivotes[i] && j<n_local ){
            sendcounts[i]++;
            j++;
        }
    }
    sendcounts[np-1]=0;
    while( j < n_local ){
        sendcounts[np-1]++;
        j++;
    }
    //if(id==0){ printf("\n[%d] sendcounts={",id);  for(i=0;i<np;i++) printf("%d, ", sendcounts[i]); printf("}\n"); } // <---- impresion
    sdispls = (int*)malloc( np*sizeof(int) );
    if (sdispls == NULL) {printf("Memoria insuficiente (sdispls) \n");MPI_Abort(MPI_COMM_WORLD, 99);}

    sdispls[0]=0;
    for(i=1;i<np;i++)
        sdispls[i] = sdispls[i-1]+sendcounts[i-1];
    //if(id==0){ printf("\n[%d] sdispls={",id);  for(i=0;i<np;i++) printf("%d, ", sdispls[i]); printf("}\n"); } // <---- impresion
    
    recvcounts =  (int*)malloc( np*sizeof(int) );
    if (recvcounts == NULL) {printf("Memoria insuficiente (recvcounts) \n");MPI_Abort(MPI_COMM_WORLD, 99);}

    MPI_Alltoall( sendcounts, 1, MPI_INT, recvcounts, 1, MPI_INT, MPI_COMM_WORLD );
    
    rdispls = (int*)malloc( np*sizeof(int) );
    if (rdispls == NULL) {printf("Memoria insuficiente (rdispls) \n");MPI_Abort(MPI_COMM_WORLD, 99);}

    rdispls[0]=0;
    for(i=1;i<np;i++)
        rdispls[i] = rdispls[i-1]+recvcounts[i-1];
    // if(id==0){ printf("\n[%d] rdispls={",id);  for(i=0;i<np;i++) printf("%d, ", rdispls[i]); printf("}\n"); } // <---- impresion

    
    ndatos_recv =0;
    for(i=0;i<np;i++)
        ndatos_recv += recvcounts[i];

    recvbuf_mezclas = (int*)malloc( ndatos_recv *sizeof(int) );
    if (recvbuf_mezclas == NULL) {printf("Memoria insuficiente (recvbuf_mezclas) \n");MPI_Abort(MPI_COMM_WORLD, 99);} 

    // Cada proceso Pi mantiene su iesima partición y envía la j-partición al proceso Pj (alltoallv)  <----- (6) 
    MPI_Alltoallv( arreglo_local, sendcounts, sdispls, MPI_INT, recvbuf_mezclas, recvcounts, rdispls, MPI_INT, MPI_COMM_WORLD );
    
    //if(id==0){ printf("\n[%d] recvbuf_mezclas={",id);  for(i=0;i<ndatos_recv;i++) printf("%d, ", recvbuf_mezclas[i]); printf("}\n"); } // <---- impresion

    // Cada proceso mezcla sus p particiones para obtener una sola lista.  <----- (7)
    int numDatos = recvcounts[0];
	for (i = 1; i < np; i++) {
			merge(recvbuf_mezclas, &numDatos, recvbuf_mezclas + rdispls[i], recvcounts[i]);
	}
  
    // Recolectar los elementos que tiene cada proceso en un solo proceso con fines de impresión <---- (8)
    MPI_Gather(&ndatos_recv, 1, MPI_INT, recvcounts, 1,MPI_INT, root, MPI_COMM_WORLD);
    if(id==root){
        rdispls[0]=0;
        for(i=1;i<np;i++)  
        rdispls[i]= rdispls[i-1] + recvcounts[i-1];
    }

    MPI_Gatherv( recvbuf_mezclas, ndatos_recv, MPI_INT, arreglo, recvcounts, rdispls, MPI_INT, root, MPI_COMM_WORLD );
      
    time = MPI_Wtime() - start;
    int ordendado = 1;
    if(id==root){ 
      for( i=1; i<n; i++ ){
          if(arreglo[i-1]>arreglo[i]){
              ordendado=0;
              break;
          }
      }
      printf("\%d, %.10f, %d  \n", np, time, ordendado);
    } // <---- impresion
    
    // Liberar memoria
    if(id==root){
        free(displs);
        free(todas_las_muestras);
        free(arreglo);
    }
    free(offset);
    free(arreglo_local);
    free(recvcounts);
    free(sendcounts);
    free(sdispls);
    free(rdispls);
    free(muestras);
    free(pivotes);

    MPI_Finalize();
    return 0;
}