#!/bin/bash

#BATCH --nodes=8            #numero de nodos (maximo 14)
# SBATCH --job-name=OpenMP    #nombre del job
# SBATCH --ntasks-per-node=6
# SBATCH --time=1:00:00       #sin limite de tiempo 
# SBATCH --partition=general  #cola de slurm


echo "Salida del programa Saludo1.c"

g++ -fopenmp paralelo.cpp -o ejecucion_paralelo.out
g++ -fopenmp secuencial.cpp -o ejecucion_secuencial.out

./ejecucion_secuencial.out 5000000  >> ./resultados.txt

for i in $(seq 1 16)
do
    echo " ----- Usando $i hilos"
   ./ejecucion_paralelo.out 5000000 $i  >> ./resultados.txt
done
