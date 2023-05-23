#include <stdio.h>
#include <stdlib.h>
#include <values.h>

#define MIN(a,b) ((a)<(b)?(a):(b) )
int main(int argc,char **argv){
    int i,j,k,m,n;
    int *adyacencia;
    FILE *arch1;

    if(argc!=3){
        printf("Uso: generador_binario <inputfilename> <outputfilename>");
        return 1;
    }
    arch1 = fopen(argv[1],"r");
    fscanf(arch1,"%d",&m);
    fscanf(arch1,"%d",&n);

    adyacencia = (int*)malloc( m*n*sizeof(int) );
    if(adyacencia==NULL){ printf("Sin memoria [adyacencia] :( "); return 1; }

    for( i = 0 ; i < m ; i++ ){
        printf("\n");
        for( j = 0 ; j < n ; j++ ){
            fscanf(arch1,"%d",&adyacencia[i*n+j]);
            if(adyacencia[i*n+j]==-1) adyacencia[i*n+j] = MAXINT;
            printf("%12d ",adyacencia[i*n+j]);
        }
    }
    fclose(arch1);

    arch1 = fopen(argv[2],"wb");
    fwrite(&m,1,sizeof(int),arch1);
    fwrite(&n,1,sizeof(int),arch1);

    fwrite(adyacencia,m*n,sizeof(int),arch1);

    fclose(arch1);

    free(adyacencia);

    return 0;
}