#!/bin/bash
# Autor: Luis Juventino Velasquez Hidalgo
# Fecha: 2/19/2023
#   Archivo para mostrar la diferencia entre los tiempos del programa

#SBATCH --nodes=1               #numero de nodos (maximo 18)
#SBATCH --job-name=OpenMP       #nombre del job
#SBATCH --ntasks-per-node=17    #maximo 40
#SBATCH --time=1:00:00          #tiempo de ejecucion (maximo 168 hrs)
#SBATCH --partition=general     #cola de slurm
#SBATCH --constraint=broadwell

#----------------------------------------------------------------------------
#------------------------ Variables de entorno ------------------------------
#----------------------------------------------------------------------------
if [[ $# == 0 ]]
then
    tamano_muestra=5000000
    numero_hilos=16
    mensaje="Ejecucion con los parametros especificados en la tarea"
    repeticion_por_hilo=10
else
    tamano_muestra=$1
    numero_hilos=$2
    mensaje=$3   
    repeticion_por_hilo=$4 
fi


#----------------------------------------------------------------------------
#------------------------ Informacion de ejecucion --------------------------
#----------------------------------------------------------------------------
# El ID  de la corrida es el time stamp
time_stamp=`date +"%d%m%Y%H%M%S"`   
#Se declara el nombre del archivo en el cual se guardaran los archivos
archivo_resultados="r$time_stamp"
#Se imprime en pantalla la informacion pertinente
echo "▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ Iniciando ejecuccion ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒"
echo " > Bitacora: $mensaje"
echo " > ID de los resultados: $time_stamp"
echo -e "\t•Tamano del experimento: $tamano_muestra"
echo -e "\t•Numero_hilos: $numero_hilos"
echo -e "\t•Repeticiones por hilo: '$repeticion_por_hilo"
echo -e "\t•Resultados en: '$archivo_resultados'"



#----------------------------------------------------------------------------
#------------------------ Compilacion de archivos ---------------------------
#----------------------------------------------------------------------------
HORA=`date +"%H:%M"`
echo -e "\v======================= Compilando archivos :: $HORA ======================="
g++ ./secuencial.cpp -fopenmp -o ejecucion_secuencial.exe
g++ ./paralelo.cpp -fopenmp -o ejecucion_paralelo.exe



#----------------------------------------------------------------------------
#-------------------------- Ejecucion algoritmos ----------------------------
#----------------------------------------------------------------------------
echo -e "\v > Inicia la toma de tiempos"
HORA=`date +"%H:%M"`
echo "--------------- corrida secuencial :: $HORA ---------------"
./ejecucion_secuencial.exe $tamano_muestra $repeticion_por_hilo >> $archivo_resultados
for i in $(seq 1 $numero_hilos)
do
    HORA=`date +"%H:%M"`
    echo "~~~~~~~~~~~~~ Ejecucion con $i hilos :: $HORA ~~~~~~~~~~~~~"
    ./ejecucion_paralelo.exe $tamano_muestra $i  $repeticion_por_hilo >> $archivo_resultados
done


#----------------------------------------------------------------------------
#----------------------------- Fin del archivo ------------------------------
#----------------------------------------------------------------------------
HORA=`date +"%H:%M"`
echo -e "░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ Ejecuccion terminada ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░"
#Creamos los plots con un archivo en python No funciona en esta cosa
#python //sshfs/ppd@148.225.111.150/juventino_velasquez_ocotillo/graficar.py $PWD $archivo_resultados $tamano_muestra
