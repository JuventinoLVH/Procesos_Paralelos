#include <stdlib.h>
#include <omp.h>
#include <stdio.h>
#include <math.h> // ceil()
#include <limits.h> // INT_MAX 


const int REPETIR = 120;
#define BLOCK_LOW(id, p, n) ((id) * (n) / (p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id) + 1, (p), (n)) - 1)
#define BLOCK_SIZE(id, p, n) (BLOCK_HIGH((id), (p), (n)) - BLOCK_LOW((id), (p), (n)) + 1)
#define MIN(i, j) (((i) < (j)) ? (i) : (j))
#define MAX(i, j) (((i) > (j)) ? (i) : (j))
//  ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
int funcionQueCompara(const void *a, const void *b) {
    // Castear a enteros
    int aInt = *(int *) a;
    int bInt = *(int *) b;
    // Al restarlos, se debe obtener un número mayor, menor o igual a 0
    // Con esto ordenamos de manera ascendente
    return aInt - bInt;
}
//  ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
void merge_mid(int *S_i, int *aux, int chunksize) {
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
        exit (1);
    }

    fscanf(myFile, "%i\n", &n); // numero de datoss a leer

    chunksize = ceil(n*1.0 / np);
    faltantes = np*chunksize - n;

    *arr = (int *) malloc((chunksize*np)*sizeof(int));
    if (*arr == NULL) {
        printf("Memoria insuficiente (arr)\n");
        exit (1);
    }

    for ( i=0; i < n; i++)
        fscanf(myFile, "%i\n", (*arr)+i);

    for ( i=n; i < n + faltantes; i++) //<--- Dummys
        (*arr)[i] = INT_MAX;

    return n;
}
//  ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
int main(int argc, char**argv) {
    int  faltantes;
    unsigned int n, chunksize, i,j,k;
    double start_time, current_time, elapsed_time; 
    int *Arreglo, *S_i, *auxiliar;

    int numThreads = atoi(argv[1]);
    omp_set_num_threads (numThreads); 

    n = read_array(argv[2], &Arreglo, numThreads);
    chunksize = ceil(n*1.0 / numThreads);
    faltantes = numThreads*chunksize - n;    

    start_time = omp_get_wtime();
    int rep;    
    #pragma omp parallel private(i,j,k,S_i,auxiliar) firstprivate(n,chunksize) shared(Arreglo,numThreads)
    {
        for (rep = 0; rep < REPETIR ; ++rep)
        {       
            int id_thread = omp_get_thread_num();
            int ind_low  = BLOCK_LOW (id_thread, numThreads, n+faltantes);
            int ind_high = BLOCK_HIGH(id_thread, numThreads, n+faltantes);
            //printf("%d)  ind_low=%d, ind_high=%d, chunksize=%d\n", id_thread,ind_low, ind_high,chunksize);


            // cada quien ordena sus chunksize valores 
            qsort(Arreglo+ind_low, chunksize, sizeof(int), funcionQueCompara);

            // Pide memoria del doble de dicha longitud para poder ordenar
            S_i = (int *) malloc(2*chunksize*sizeof(int));  if (S_i == NULL) {   printf("Memoria insuficiente (S_i)\n");exit (1); }
            auxiliar = (int *) malloc(2*chunksize*sizeof(int));  if (auxiliar == NULL) {   printf("Memoria insuficiente (auxiliar)\n");exit (1); }

            // cada quien copia en S_i sus chunksize valores
            for( i = ind_low, j=0 ; i <= ind_high ; i++, j++)
                S_i[j] = Arreglo[i];       

            for(i=1;i<=ceil(numThreads/2.0);i++){
                if(id_thread%2==0 && id_thread!=numThreads-1) {   
                    for( k = ind_high+1, j = chunksize ; j<2*chunksize ; k++,j++ ){
                        S_i[j] = Arreglo[k];
                    // printf("%d, ",S_i[j]);
                    }

                    merge_mid(S_i, auxiliar, chunksize); 

                    // actualizar Arreglo 
                    for( k = ind_low, j=0; j < 2*chunksize ; k++, j++ )
                        Arreglo[k] = S_i[j] ;
                }

                #pragma omp barrier            
                for( k = ind_low, j=0; j < chunksize ; k++, j++ )
                    S_i[j] = Arreglo[k] ;

                if(id_thread%2==1 && id_thread!=numThreads-1){
                    for(k=ind_high+1, j = chunksize; j < 2*chunksize ; k++, j++)
                        S_i[j] = Arreglo[k];

                    merge_mid(S_i,auxiliar, chunksize); 
                    
                    // actualizar Arreglo 
                    j = 0;
                    for( k = ind_low, j=0; j < 2*chunksize ; k++, j++ )
                        Arreglo[k] = S_i[j] ;
                }
                #pragma omp barrier
                for( k = ind_low, j=0; j < chunksize ; k++, j++ )
                    S_i[j] = Arreglo[k] ;  
            }        
        }
    }
    current_time = omp_get_wtime();                     //■■■■■■■■■■■■■■■■■■■■■■■ <--- toma de tiempo 2

    elapsed_time = current_time - start_time; 
    printf("%i,  %.10f \n",numThreads, elapsed_time); 

    /*printf("\nArreglo = {");
    for(i=0;i<n;i++)
        printf("%d, ", Arreglo[i]);
    printf("}");*/

    return elapsed_time;
}