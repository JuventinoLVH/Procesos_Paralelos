#!/bin/bash
# Autor: Luis Juventino Velasquez Hidalgo
# Fecha: 2/19/2023
#   Archivo para mostrar la diferencia entre los tiempos del programa 
#   merge-split con diferentes hilos

#SBATCH --nodes=5           
#SBATCH --job-name=OpenMP    
#SBATCH --ntasks-per-node=4
#SBATCH --time=1:00:00
#SBATCH --partition=general

echo "Corriendo el programa:"
tamano_muestra=$1

time_stamp=`date +"%d%m%Y%H%M%S"`
archivo_muestra="muestra_$tamano_muestra"
archivo_resultados="resultados$time_stamp"

#gcc ./secuencial.c -fopenmp -o ejecucion_secuencial.exe
#gcc ./paralelo.c -fopenmp -o ejecucion_paralelo.exe
#gcc ./generar_numeros.c -fopenmp -o generar_numeros.exe

./generar_numeros.exe $archivo_muestra $tamano_muestra
./ejecucion_secuencial.exe $archivo_muestra >> $archivo_resultados

for i in $(seq 1 4)
do
    echo " --- Ejecucion con $i hilos"
    ./ejecucion_paralelo.exe $i $archivo_muestra >> $archivo_resultados
done
