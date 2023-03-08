#!/bin/bash

#SBATCH --nodes=1               #numero de nodos (maximo 18)
#SBATCH --job-name=OpenMP       #nombre del job
#SBATCH --ntasks-per-node=20  #maximo 40
#SBATCH --time=1:00:00          #tiempo de ejecucion (maximo 168 hrs)
#SBATCH --partition=general     #cola de slurm
#SBATCH --constraint=broadwell


# Autor: Luis Juventino Velasquez Hidalgo
# Fecha: 2/19/2023
#   Archivo para mostrar la diferencia entre los tiempos del programa 
#   criba_max_gap con diferentes hilos
#   Primer parametro: tamano de muestra
#   Segundo parametro: numero de hilos a usar
#   Tercer parametro: Una bandera que dice si se crean los ejecutables 


#Variables de entorno...
tamano_muestra=$1
numero_hilos=$2
mensaje=$3

time_stamp=`date +"%d%m%Y%H%M%S"`
archivo_resultados="resultados$time_stamp"



# Solamente compilamos si se lo pedmios desde bach
if [[ $# > 2 ]]
then
    g++ ./secuencial.c -fopenmp -o ejecucion_secuencial.exe
    g++ ./paralelo.c -fopenmp -o ejecucion_paralelo.exe
    g++ ./generar_numeros.c -fopenmp -o generar_numeros.exe
fi



#Corremos los programas y tomamos tiempo
echo "Corriendo el programa ..."
echo "Capsula: $mensaje "
echo "Tamano del experimento: $tamano_muestra"
echo "numero_hilos: $numero_hilos"
echo "ID de los resultados: $time_stamp"

DIA=`date +"%d/%m/%Y"`
HORA=`date +"%H:%M"`

archivo_con_numeros="$tamano_muestra_$time_stamp"
echo "Generando numeros..."
./generar_numeros.exe $archivo_con_numeros $tamano_muestra

echo "La corrida inicio a las: $DIA $HORA!"
./ejecucion_secuencial.exe $archivo_con_numeros >> $archivo_resultados
for i in $(seq 1 $numero_hilos)
do
    echo " --- Ejecucion con $i hilos"
    ./ejecucion_paralelo.exe $i $archivo_con_numeros >> $archivo_resultados
done


DIA=`date +"%d/%m/%Y"`
HORA=`date +"%H:%M"`
echo "La corrida termino a las: $DIA $HORA!"
#Creamos los plots con un archivo en python No funciona en choya :( 
#python ./graficar.py $PWD $archivo_resultados $tamano_muestra




