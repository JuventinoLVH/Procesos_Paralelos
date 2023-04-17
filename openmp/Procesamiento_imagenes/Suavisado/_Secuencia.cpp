#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "Cronometro.h"

  
int main(int argc,char *argv[]){

    int i, j;    
    int width = 500, height = 255;
    int matrix[width][height] ;

    for(int i=0;i<height;i++)
        for(int j=0;j<width;j++)
            matrix[i][j]= i;
  
    FILE* pgmFile;
    pgmFile = fopen(argv[1], "wb");
    if (pgmFile == NULL) {
        std::cout<<("Cannot open file to write\n");
        system("pause");
        return 1;
    }

    fprintf(pgmFile, "P5 ");
    fprintf(pgmFile, "%d %d ", width, height);
    fprintf(pgmFile, "%d ", 255);

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            fputc(matrix[i][j], pgmFile);
        }
    }

    fclose(pgmFile);
    system("pause");    
    return  0;
}