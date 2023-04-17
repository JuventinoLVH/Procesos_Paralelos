#!/bin/bash

#SBATCH --nodes=5           
#SBATCH --job-name=OpenMP    
#SBATCH --ntasks-per-node=4
#SBATCH --time=1:00:00
#SBATCH --partition=general


# Autor: Luis Juventino Velasquez Hidalgo
# Fecha: 2/19/2023
#   Archivo para mostrar la diferencia entre los tiempos del programa 
#   criba_max_gap con diferentes hilos


#Variables de entorno...
tamano_muestra=$1
numero_hilos=$2
time_stamp=`date +"%d%m%Y%H%M%S"`
archivo_resultados="resultados$time_stamp"



# Solamente compilamos si se lo pedmios desde bach
if [[ $# > 2 ]]
then
    g++ ./secuencial.cpp -fopenmp -o ejecucion_secuencial.exe
    g++ ./paralelo.cpp -fopenmp -o ejecucion_paralelo.exe
fi



#Corremos los programas y tomamos tiempo
echo "Corriendo el programa. ID de los resultados: $time_stamp"

DIA=`date +"%d/%m/%Y"`
HORA=`date +"%H:%M"`
echo "La corrida inicio a las: $DIA $HORA!"

./ejecucion_secuencial.exe $tamano_muestra >> $archivo_resultados
for i in $(seq 1 $numero_hilos)
do
    echo " --- Ejecucion con $i hilos"
    ./ejecucion_paralelo.exe $tamano_muestra $i  >> $archivo_resultados
done


DIA=`date +"%d/%m/%Y"`
HORA=`date +"%H:%M"`
echo "La corrida termino a las: $DIA $HORA!"
#Creamos los plots con un archivo en python No funciona en choya :( 
#python ./graficar.py $PWD $archivo_resultados $tamano_muestra




