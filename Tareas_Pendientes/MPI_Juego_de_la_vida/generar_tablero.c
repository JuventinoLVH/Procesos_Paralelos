#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    int n, m;
    FILE *file;
    char *filename;
    int i, j;

    // Comprobamos que se pasaron los argumentos necesarios
    if (argc < 4) {
        printf("Uso: %s n m archivo_salida\n", argv[0]);
        return 1;
    }

    // Obtenemos los valores de n, m y el nombre de archivo de los argumentos
    n = atoi(argv[1]);
    m = atoi(argv[2]);
    filename = argv[3];

    // Abrimos el archivo de salida
    file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error al abrir el archivo %s\n", filename);
        return 1;
    }

    // Escribimos los valores de n y m en las primeras líneas del archivo
    fprintf(file, "%d %d\n", n, m);

    // Generamos la matriz aleatoria
    srand(time(NULL));  // Inicializamos la semilla para los números aleatorios
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            // Generamos un valor aleatorio de 0 o 1
            int value = rand() % 2;
            // Escribimos el valor en el archivo
            fprintf(file, "%d ", value);
        }
        fprintf(file, "\n"); // Cambiamos de línea al terminar la fila
    }

    // Cerramos el archivo y salimos
    fclose(file);
    return 0;
}
