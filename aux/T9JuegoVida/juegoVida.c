#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "MyMPI.h"

/* Change these two definitions when the matrix and vector
   element types change */

typedef int dtype;
#define mpitype MPI_INT

void game_of_life_iteration(dtype **a, int m, int n, MPI_Comm comm) {
    int id, np;
    int i, j;

    MPI_Comm_rank(comm, &id);
    MPI_Comm_size(comm, &np);

    int rows = BLOCK_SIZE(id, np, m);

    //Matriz temporal para guardar el tablero original. Se hace + 2 por los
    // renglones que se reciben del proceso anterior y del siguiente.
    dtype **temp = (dtype **) malloc((rows + 2) * sizeof(dtype *));

    // Alocar memoria a temp
    for (i = 0; i < rows + 2; i++) {
        temp[i] = (dtype *) malloc(n * sizeof(dtype));
    }

    // Copian los renglones del tablero a en temp
    for (i = 1; i <= rows; i++) {
        memcpy(temp[i], a[i - 1], n * sizeof(dtype));
    }
    
    // Se manda el último renglón de cada proceso (excepto el ultimo) al siguiente.
    if (id < np - 1) {
        MPI_Send(a[rows - 1], n, mpitype, id + 1, 0, comm);
    }
    
      MPI_Barrier(MPI_COMM_WORLD);
    // Recive el renglon del proceso anterior y lo guarda en el primero.
    if (id > 0) {
        MPI_Recv(temp[0], n, mpitype, id - 1, 0, comm, MPI_STATUS_IGNORE);
    }

    // Se manda el primer renglón de cada proceso (excepto el primer) al anterior.
    if (id > 0) {
        MPI_Send(a[0], n, mpitype, id - 1, 0, comm);
    }

      MPI_Barrier(MPI_COMM_WORLD);
    // Recive el renglon del proceso siguiente y lo guarda en el último.
    if (id < np - 1) {
        MPI_Recv(temp[rows + 1], n, mpitype, id + 1, 0, comm, MPI_STATUS_IGNORE);
    }

    // Crear una nueva matriz para guardar los resultados.
    dtype **a_aux = (dtype **) malloc(rows * sizeof(dtype *));
    for (i = 0; i < rows; i++) {
        a_aux[i] = (dtype *) malloc(n * sizeof(dtype));
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // Se realiza una iteración del juego de la vida y se guarda en 'a_aux'
    int neighbors;
    for (i = 1; i <= rows; i++) {
        for (j = 0; j < n; j++) {
            neighbors = 0;
            if (i == 1 && j == 0 ) { // esquina superior izquierda (3 vecinos)
                if(id == np-1 && rows == 1) {neighbors = temp[0][0] + temp[0][1] + temp[1][1];}
                  else {
                    neighbors = temp[1][1] + temp[2][0] + temp[2][1];
                    if(np > 1 && id != 0) neighbors += temp[0][0] + temp[0][1];
                  }
            } else if (i == 1 && j == n-1) { // esquina superior derecha (3 vecinos)
                if(id == np-1 && rows == 1){ neighbors = temp[0][n-2] + temp[0][n-1] + temp[1][n-2];}
                else {
                  neighbors = temp[1][n-2] + temp[2][n-2] + temp[2][n-1];
                  if(np > 1 && id != 0) neighbors += temp[0][n-1] + temp[0][n-2];
                }
            } else if (i == rows && j == 0) { // esquina inferior izquierda (3 vecinos)
                neighbors = temp[rows-1][0] + temp[rows-1][1] + temp[rows][1];
                if(np > 1 && id < np-1) neighbors += temp[rows+1][0] + temp[rows+1][1];
            } else if (i == rows && j == n-1) { // esquina inferior derecha (3 vecinos)
                neighbors = temp[rows-1][n-2] + temp[rows-1][n-1] + temp[rows][n-2];
                if(np > 1 && id < np-1) neighbors += temp[rows+1][n-1] + temp[rows+1][n-2];
            } else if (i == 1 && j != 0 && j != n-1) { // primer renglon, diferente a las esquinas (5 vecinos)
                if(id == np-1 && rows == 1) {neighbors = temp[i][j-1] + temp[i][j+1] + temp[i-1][j-1] + temp[i-1][j] + temp[i-1][j+1];}
                else {
                  neighbors = temp[1][j-1] + temp[1][j+1] + temp[2][j-1] + temp[2][j] + temp[2][j+1];
                  if(np > 1 && id != 0) neighbors += temp[0][j-1] + temp[0][j] + temp[0][j+1];
                }
            } else if (i == rows && j != 0 && j != n-1) { // último renglon, diferente a las esquinas (5 vecinos)
                neighbors = temp[i][j-1] + temp[i][j+1] + temp[i-1][j-1] + temp[i-1][j] + temp[i-1][j+1];
                if(np > 1 && id < np-1) neighbors += temp[rows+1][j-1] + temp[rows+1][j] + temp[rows+1][j+1];
            } else if (j == 0 && i != 1 && i != rows) { // primer columna, diferente a las esquinas (5 vecinos)
                neighbors = temp[i-1][j] + temp[i+1][j] + temp[i-1][j+1] + temp[i][j+1] + temp[i+1][j+1];
            } else if (j == n-1 && i != 1 && i != rows) { // última columna, diferente a las esquinas (5 vecinos)
                neighbors = temp[i-1][j] + temp[i+1][j] + temp[i-1][j-1] + temp[i][j-1] + temp[i+1][j-1];
            } else { // todas las celdas interiores (8 vecinos)
                neighbors = temp[i-1][j-1] + temp[i-1][j] + temp[i-1][j+1] + temp[i][j-1] + temp[i][j+1] + temp[i+1][j-1] + temp[i+1][j] + temp[i+1][j+1];
            }
          

            if (temp[i][j] == 1) {
                if (neighbors < 2 || neighbors > 3) {
                    a_aux[i - 1][j] = 0; // La celula se muere
                } else {
                    a_aux[i - 1][j] = 1; // La celula sobrevive
                }
            } else {
                if (neighbors == 3) {
                    a_aux[i - 1][j] = 1; // La celula sobrevive
                } else {
                    a_aux[i - 1][j] = 0; // La celula se queda muerta
                }
            }
        }
    }

    // Reemplaza la matriz original 'a' con los valores nuevos de 'a_aux'
    for (i = 0; i < rows; i++) {
        memcpy(a[i], a_aux[i], n * sizeof(dtype));
        free(a_aux[i]);
    }
    free(a_aux);
    MPI_Barrier(MPI_COMM_WORLD);

    // Liberea memoria temporal
    for (i = 0; i < rows + 2; i++) {
        free(temp[i]);
    }
    free(temp);

}

int main (int argc, char *argv[]) {
    dtype **a;  // Matriz
    dtype *storage;  /* Matrix elements stored here */
    int    i, j, id, np, n, m ;     /* Loop indices */
    int    nprime;   /* Elements in vector */
    int    its;

    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &id);
    MPI_Comm_size (MPI_COMM_WORLD, &np);

    // Lee la cantidad de iteraciones, y k (cada que cantidad de iteraciones se imprime)
    int iterations = atoi(argv[2]);
    int k = atoi(argv[3]);
    if(!id && k > iterations){
      printf("El numero k (impresiones) es  mayor al numero de iteraciones!");
      return 1;
    }

    //Se utiliza las función de read_row_striped_matrix para leer el archivo matriz y separar los renglones entre los proccesos.
    read_row_striped_matrix(argv[1], (void *) &a, (void *) &storage, mpitype, &m, &n, MPI_COMM_WORLD);
    
    if(!id) printf("\nTablero Inicial\n");
    print_game((void **) a, mpitype, m, n, MPI_COMM_WORLD);

    j = 0; // contador auxiliar para k
    for (int i = 0; i < iterations; i++) {
        game_of_life_iteration(a, m, n, MPI_COMM_WORLD); //Se realiza una iteracion del juego de la vida.
        j++;

        if(j == k){
          if(!id) printf("Iteracion #%d \n", i+1);
          print_game((void **)a, mpitype, m, n, MPI_COMM_WORLD);
          j = 0;
        }
    }

    free(a);
    MPI_Finalize();
    return 0;
}