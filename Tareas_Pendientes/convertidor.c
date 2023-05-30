#include <stdio.h>
#include <stdlib.h>
#include <values.h>

int main( int argc, char **argv ){
    int i, j, k, m;
    int *adyacencia;
    FILE *arch1;

    if( argc != 3 ){
        printf("\nUso: generador_binario <inputfilename> <outputfilename>");
        return 1;
    }

    arch1 = fopen(argv[1],"r");
    if(!arch1){ printf("\nERROR: El archivo %s no se pudo abrir para lectura ",argv[1]);    return 2;   }
        fscanf(arch1,"%d",&m);

        printf("\nm=%d",m);

        adyacencia = (int*)malloc( m*sizeof(int) );
        if( adyacencia == NULL ){   printf("\nSin memoria :( ");    return 1;  }

        for( i = 0 ; i < m ; i++ ){
            fscanf(arch1,"%d",&adyacencia[i]);
            printf("%12d ",adyacencia[i]);
        }
    fclose(arch1);


    arch1=fopen(argv[2],"wb");
    if(!arch1){ printf("\nERROR: El archivo %s no se pudo abrir para escritura ",argv[2]);    return 2;   }
        fwrite(&m,1,sizeof(int),arch1);

        fwrite(adyacencia,m,sizeof(int),arch1);
    fclose(arch1);

    free(adyacencia);

    return 0;
}
