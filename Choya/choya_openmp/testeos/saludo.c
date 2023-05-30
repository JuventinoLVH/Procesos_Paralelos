#include <stdio.h>
#include <omp.h>

int main(){
	#pragma omp parallel
	{
		print("Hilo %d: Hola mundo de OPENMP!\n",omp_get_thread_num());
	}
	return 0;
}
