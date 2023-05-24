#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


//  ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
int* merge(int *bloqueA,int *bloqueB,unsigned int nA,unsigned int nB);
int compare(const void *_a, const void *_b);
void separarSublistas_Pivote(int *bloque, unsigned int *cantMen, int **subMenor, unsigned int *cantMay, int **subMayor, unsigned int n, int pivote, int id, int iteracion);
int read_array(char* fname, int **arr, int np) ;

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
    root = 0;
    if (id == root) { // Lee los datos del archivo
        n = read_array(argv[1], &datos, np);
    }

    MPI_Cart_create( MPI_COMM_WORLD, ND, dimensiones, periodicos, 1, &comm_topologia_hipercubo );

    if( comm_topologia_hipercubo != MPI_COMM_NULL ){
        MPI_Comm_rank(comm_topologia_hipercubo, &idCart);
        MPI_Comm_size(comm_topologia_hipercubo, &pCart);

        MPI_Bcast(&n, 1, MPI_UNSIGNED, root, comm_topologia_hipercubo);
        //printf("Distribucion \n");
        //  --Distribución de los datos--
        n_local = BLOCK_SIZE(idCart,pCart,n);
        
        bloque = (int *) malloc(n_local*sizeof(int)); if (bloque == NULL) {printf("Memoria insuficiente (bloque)\n");MPI_Abort(MPI_COMM_WORLD, 99);}
        sendcounts = (unsigned int*) malloc(pCart*sizeof(unsigned int));    if( sendcounts == NULL ){ printf("ERROR: Memoria insuficiente! (sendcounts)\n");    MPI_Abort(comm_topologia_hipercubo, 99);  }
        sdispls = (unsigned int*) malloc(pCart*sizeof(unsigned int));    if( sdispls    == NULL ){ printf("ERROR: Memoria insuficiente! (sdispls)\n");       MPI_Abort(comm_topologia_hipercubo, 99); }

        if(idCart == root){
            for( i = 0 ; i < pCart; i++ )
                sendcounts[i]=BLOCK_SIZE(i,pCart,n);
            
            sdispls[0] = 0;
            for( i = 1 ; i < pCart; i++ )  
                sdispls[i] = sdispls[i-1] + sendcounts[i-1];
        }

        MPI_Scatterv( datos, sendcounts, sdispls, MPI_INT, bloque, n_local, MPI_INT, root, comm_topologia_hipercubo );

        //  --Ordenamiento QSort Local--
        qsort(bloque, n_local, sizeof(int), compare);
        
        //  --Selección de pivote Mediana--
        if(idCart == root){
            if(n_local % 2 == 0) pivote = (bloque[n_local/2]+bloque[(n-2)/2])/2; //Promedio de los dos valores centrales
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
    
    free(bloque);
    free(displs);
    free(dimensiones);
    free(periodicos);
    free(cambiosDedimension);
    free(cambiosDedimension2);

    MPI_Finalize();
    return 0;

}




//  ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
int read_array(char* fname, int **arr, int np) {
    FILE *myFile;
    unsigned int i, n, chunksize, faltantes;

    myFile = fopen(fname, "r");
    if(!myFile){
        printf("ERROR: No se pudo abrir el archivo %s",fname);
        MPI_Abort(MPI_COMM_WORLD, 99);
    }
    fscanf(myFile, "%i\n", &n); // numero de datoss a leer
    *arr = (int *) malloc( n * sizeof(int) );
    if (*arr == NULL) {
        printf("Memoria insuficiente\n");
        MPI_Abort(MPI_COMM_WORLD, 99);
    }
    for ( i=0; i < n; i++)
        fscanf(myFile, "%i\n", (*arr)+i);

    return n;
}

//  ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
int compare(const void *_a, const void *_b) { 
    int *a, *b;
    
    a = (int *) _a;
    b = (int *) _b;
    return (*a - *b);
}

//  ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
void separarSublistas_Pivote(int *bloque, unsigned int *cantMen, int **subMenor, unsigned int *cantMay, int **subMayor, unsigned int n, int pivote, int id, int iteracion){
	int i;
    *subMenor = NULL;
    *subMayor = NULL;

    *cantMen = 0;
    i=0;
    for(i = 0; i < n; i++) {
        if(bloque[i] <= pivote)
            (*cantMen)++;
    }

    if(*cantMen){
        *subMenor =(int *)malloc((*cantMen)*sizeof(int));  if(*subMenor==NULL){printf("\n(%d) ERROR: No hay memoria (*subMenor)\n\n",iteracion);  MPI_Abort(MPI_COMM_WORLD, 99);}
    
        for(i=0;i<*cantMen;i++)
            (*subMenor)[i] = bloque[i];
    }
    
    *cantMay = n-(*cantMen);
    if(*cantMay){
        *subMayor =(int *)malloc((*cantMay)*sizeof(int));  if(*subMayor==NULL){printf("\n(%d) ERROR: No hay memoria (*subMayor)\n\n",iteracion);  MPI_Abort(MPI_COMM_WORLD, 99);}

        for(i=*cantMen;i<n;i++)
            (*subMayor)[i-*cantMen]=bloque[i];
    }

	return;
}
//  ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
int* merge(int *bloqueA,int *bloqueB,unsigned int nA,unsigned int nB){
    unsigned int a = 0, b = 0, i = 0;
    int *aux;
    aux = (int *)malloc((nA+nB)*sizeof(int));   if(aux==NULL){printf("\n ERROR: No hay memoria (aux)\n\n");  MPI_Abort(MPI_COMM_WORLD, 99);}

    while (a < nA && b < nB){ aux[i++] = (bloqueA[a] < bloqueB[b]) ? bloqueA[a++] : bloqueB[b++]; }
    
    while (a < nA){ aux[i++] = bloqueA[a++]; }
    
    while (b < nB){ aux[i++] = bloqueB[b++]; }    

    return aux;
}