#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "HiperQS_Header.h"

#define BLOCK_LOW(id,p,n) ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id)+1,p,n)-1)
#define BLOCK_SIZE(id,p,n)(BLOCK_HIGH(id,p,n)-BLOCK_LOW(id,p,n)+1)
//  ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

int main(int argc,char *argv[]){
    int id, np, root;
    int partner, pivote, *datos, *bloque, *bloqueMen, *bloqueMay, *bloqueAux;
    int  ND, *dimensiones, *periodicos;
    int nprocesos_minimo, idCart, idCart2, pCart, ordenado;
    int  *cambiosDedimension, *cambiosDedimension2; 
    unsigned int cantMen, cantMay, cantMenRecv, cantMayRecv;
    unsigned int n, i, j, n_local, *displs, *sendcounts,*recvcounts,*sdispls, ndatos_recv;
    MPI_Comm comm_topologia_hipercubo, comm_sub_hipercubo, comm_sub_hipercubo2;
    double start, time;
    MPI_Offset  *offset;
    MPI_Status  status;
    MPI_File fh;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &np); // Numero total de procesos
    MPI_Comm_rank(MPI_COMM_WORLD, &id); // Valor de nuestro identificador

    MPI_Barrier(MPI_COMM_WORLD);  // sincronización
    start = MPI_Wtime();

    ND = atoi(argv[2]);

    dimensiones = (int*) malloc(ND*sizeof(int));    if( dimensiones == NULL ){ printf("ERROR: Memoria insuficiente! (dimensiones)\n");  MPI_Abort(MPI_COMM_WORLD, 99); }
    periodicos = (int*) malloc(ND*sizeof(int));     if( periodicos  == NULL ){ printf("ERROR: Memoria insuficiente! (periodicos)\n");   MPI_Abort(MPI_COMM_WORLD, 99); }

    nprocesos_minimo = 1; 
    for ( i = 0 ; i < ND ; i++ ){
        dimensiones[i] = 2;
        periodicos[i]  = 0;
        nprocesos_minimo *= 2;
    }

    if( np < nprocesos_minimo ){
        printf( "\nERROR: Con dimension %d, Se requieren al menos %d procesos \n\n", ND, nprocesos_minimo );
        MPI_Abort( MPI_COMM_WORLD, 2 );
    }
    //printf("Leer \n");

    MPI_Cart_create( MPI_COMM_WORLD, ND, dimensiones, periodicos, 1, &comm_topologia_hipercubo );

    if( comm_topologia_hipercubo != MPI_COMM_NULL ){
        MPI_Comm_rank(comm_topologia_hipercubo, &idCart);
        MPI_Comm_size(comm_topologia_hipercubo, &pCart);
        root = 0;

        offset = (MPI_Offset*)malloc(np*sizeof(MPI_Offset));if(offset==NULL){ printf("\n ERROR: No hay memoria suficiente (offset)"); MPI_Abort(MPI_COMM_WORLD, 99); }
        MPI_File_open( comm_topologia_hipercubo, argv[1], MPI_MODE_RDONLY, MPI_INFO_NULL  , &fh );
            MPI_File_read_all(fh, &n, 1, MPI_INT, &status);

            offset[0] = sizeof(int); // <--- se salta el valor n 
            for( i = 1 ; i < np ; i++ ) 
                offset[i] = offset[i-1] + BLOCK_SIZE( i-1, np, n )*sizeof(int);

            n_local = BLOCK_SIZE( idCart, pCart, n );

            bloque = (int*)malloc(n_local*sizeof(int)); if( bloque == NULL ){ printf("\n ERROR: No hay memoria suficiente (buffer)"); MPI_Abort(MPI_COMM_WORLD, 99); }
            
            MPI_File_read_at_all(fh, offset[id], bloque, n_local, MPI_INT, &status); 

        MPI_File_close(&fh);
        datos = (int *) malloc( n * sizeof(int) );
        if (datos == NULL) {
            printf("Memoria insuficiente (datos)\n");
            MPI_Abort(MPI_COMM_WORLD, 99);
        }
            

        //  --Ordenamiento QSort Local--
        qsort(bloque, n_local, sizeof(int), &compare);
        
        
        //  --Selección de pivote Mediana--
        if(idCart == root){
            if(n_local % 2 == 0) pivote = (bloque[n_local/2]+bloque[(n_local-2)/2])/2; //Promedio de los dos valores centrales
            else pivote = bloque[n_local/2];
        }
        
        //  --Broadcast--
        MPI_Bcast(&pivote, 1, MPI_INT, root, comm_topologia_hipercubo );
        
        //printf("Separacion \n");
        //  --Separación de datos en dos Sublistas--
        separarSublistas_Pivote(bloque, &cantMen, &bloqueMen, &cantMay, &bloqueMay, n_local, pivote, idCart, 0);

        cambiosDedimension  = (int*) malloc(ND*sizeof(int));  if( cambiosDedimension  == NULL ){ printf("ERROR: Memoria insuficiente! (cambiosDedimension)\n"); MPI_Abort(MPI_COMM_WORLD, 99); }
        cambiosDedimension2 = (int*) malloc(ND*sizeof(int));  if( cambiosDedimension2 == NULL ){ printf("ERROR: Memoria insuficiente! (cambiosDedimension2)\n"); MPI_Abort(MPI_COMM_WORLD, 99); }
        
        for( i = 0 ; i < ND; i++ ){
            for(j=0;j<ND;j++){
                if(j==i){
                    cambiosDedimension[j]=1;
                    cambiosDedimension2[j]=0;
                }else{
                    cambiosDedimension[j]=0;
                    if(i>j) cambiosDedimension2[j]=0;
                    else cambiosDedimension2[j]=1;
                }
            }
            MPI_Cart_sub(comm_topologia_hipercubo, cambiosDedimension, &comm_sub_hipercubo);
            MPI_Comm_rank(comm_sub_hipercubo, &idCart); 

            if(idCart==0){
                //  --Determinar Partner--
                partner = 1;
                //  --Se Mandan Mayores--
                MPI_Send( &cantMay, 1, MPI_UNSIGNED, partner, 0, comm_sub_hipercubo);
                if(cantMay != 0){
                    MPI_Send(bloqueMay, cantMay, MPI_INT, partner, 3, comm_sub_hipercubo);
                    free(bloqueMay);
                }
                //  --Se Reciven Menores--
                MPI_Recv( &cantMenRecv, 1, MPI_UNSIGNED, partner, 1, comm_sub_hipercubo, MPI_STATUS_IGNORE );
                if( cantMenRecv != 0 ){
                    bloqueAux=(int *)malloc(cantMenRecv*sizeof(int)); if( bloqueAux  == NULL ){ printf("ERROR: Memoria insuficiente! (bloqueAux)\n"); MPI_Abort(MPI_COMM_WORLD, 99); }
                    MPI_Recv(bloqueAux, cantMenRecv, MPI_INT, partner, 4, comm_sub_hipercubo, MPI_STATUS_IGNORE);
                }

                if(n_local != 0) free(bloque);

                //  --Mezcla de Sublistas--
                bloque = merge(bloqueMen, bloqueAux, cantMen, cantMenRecv);
                
                n_local = cantMen + cantMenRecv;
                if(cantMen  != 0) free(bloqueMen);
                if(cantMenRecv != 0) free(bloqueAux);
            }else{
                //  --Determinar Partner--
                partner = 0;
                //  --Se Mandan Menores--
                MPI_Send(&cantMen,  1, MPI_UNSIGNED, partner, 1, comm_sub_hipercubo);
                if(cantMen!=0){
                    MPI_Send(bloqueMen, cantMen, MPI_INT, partner, 4, comm_sub_hipercubo);
                    free(bloqueMen);
                }
                //  --Se Reciven Mayores--
                MPI_Recv(&cantMayRecv, 1, MPI_UNSIGNED, partner, 0, comm_sub_hipercubo,  MPI_STATUS_IGNORE);
                if(cantMayRecv!=0){
                    bloqueAux=(int *)malloc(cantMayRecv*sizeof(int)); if( bloqueAux  == NULL ){ printf("ERROR: Memoria insuficiente! (bloqueAux)\n"); MPI_Abort(MPI_COMM_WORLD, 99); }	
                    MPI_Recv(bloqueAux, cantMayRecv, MPI_INT, partner, 3, comm_sub_hipercubo,  MPI_STATUS_IGNORE);
                }
                if( n_local != 0 ) free(bloque);
                //  --Mezcla de Sublistas--
                bloque = merge(bloqueMay, bloqueAux, cantMay, cantMayRecv);
                
                n_local = cantMay + cantMayRecv;
                if( cantMay  != 0 ) free(bloqueMay);
                if( cantMayRecv != 0 ) free(bloqueAux);
            }

            if(i != ND-1){
                MPI_Cart_sub(comm_topologia_hipercubo, cambiosDedimension2, &comm_sub_hipercubo2);
                MPI_Comm_rank(comm_sub_hipercubo2, &idCart2);
                if(idCart2==0){
                    if(n_local > 0){
                        if(n_local%2==0) pivote = (bloque[n_local/2]+bloque[(n_local-2)/2])/2; //Promedio de los dos valores centrales
                        else pivote = bloque[n_local/2];
                    }else{
                        printf("\n ERROR: El root se quedó sin datos \n\n");
                        MPI_Abort(MPI_COMM_WORLD, 99);
                    }
                }
                MPI_Bcast(&pivote,1,MPI_INT,0,comm_sub_hipercubo2);	
                separarSublistas_Pivote(bloque, &cantMen, &bloqueMen, &cantMay, &bloqueMay, n_local, pivote, idCart, i);
            }
        }
        if(ND>1){
            MPI_Comm_free(&comm_sub_hipercubo);
            MPI_Comm_free(&comm_sub_hipercubo2);
        }

        MPI_Comm_rank(comm_topologia_hipercubo, &idCart);
        MPI_Comm_size(comm_topologia_hipercubo, &pCart);

        if(idCart==root){
            recvcounts = (unsigned int*) malloc(pCart*sizeof(unsigned int));  if(recvcounts  == NULL){ printf("ERROR: Memoria insuficiente! (recvcounts)\n"); MPI_Abort(MPI_COMM_WORLD, 99); }
        }

        MPI_Gather( &n_local, 1,   MPI_UNSIGNED, recvcounts, 1, MPI_UNSIGNED, root , comm_topologia_hipercubo );
        displs = (unsigned int*)malloc(pCart*sizeof(unsigned int)); if(displs == NULL){ printf("ERROR: Memoria insuficiente (displs)"); MPI_Abort(MPI_COMM_WORLD,99);}
        if(idCart==root){
            displs[0] = 0; 
            for( i = 1 ; i < pCart ; i++ ) 
                displs[i] = displs[i-1] + recvcounts[i-1]; 
        }
        
        MPI_Gatherv(bloque, n_local, MPI_INT, datos, recvcounts, displs, MPI_INT, root, comm_topologia_hipercubo);
        //printf("Gather \n");
        time = MPI_Wtime() - start;
        ordenado = 1;
        if(idCart==root){
            //printf("Entro \n");
            for( i=1; i<n; i++ ){
                if(datos[i-1]>datos[i]){
                    ordenado=0;
                    break;
                }
                //printf("%d, ", datos[i]);
            }
            printf("\%d, %.10f, %d  \n", np, time, ordenado);
        } //  --Salida--
        MPI_Comm_free(&comm_topologia_hipercubo);
    
    }

    
    //  --Liberar Memoria--
    if(id==root){
        free(recvcounts);
    }
    free(offset);
    free(bloque);
    free(displs);
    free(dimensiones);
    free(periodicos);
    free(cambiosDedimension);
    free(cambiosDedimension2);
    
    

    MPI_Finalize();
    return 0;

}