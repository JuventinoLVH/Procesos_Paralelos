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