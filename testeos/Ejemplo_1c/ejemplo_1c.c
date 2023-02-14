#include <stdio.h>
#include <omp.h>

int main( ){    
	int t = omp_get_num_procs(); //<-- numero de procesadores disponibles 
	omp_set_num_threads(2*t);
	printf("Núm. de procesadores fisicos disponibles = %d\nNúmero de hilos utilizados = %d\n",t, 2*t);

	#pragma omp parallel
	{
		printf("Soy el hilo %d, Hola mundo de OPENMP!\n", omp_get_thread_num());
	}
 
	return 0;
}
