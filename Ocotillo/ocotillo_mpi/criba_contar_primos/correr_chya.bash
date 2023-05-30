#!/bin/bash
# Autor: Luis Juventino Velasquez Hidalgo
# Fecha: 2/19/2023
#   Archivo para mostrar la diferencia entre los tiempos del programa

#SBATCH --nodes=4        
#SBATCH --job-name=OpenMP    
#SBATCH --ntasks-per-node=4
#SBATCH --time=1:00:00
#SBATCH --partition=general

module load intel/oneAPI-2021
#----------------------------------------------------------------------------
#------------------------ Variables de entorno ------------------------------
#----------------------------------------------------------------------------
if [[ $# != 3 ]]
then
    mensaje="Ejecucion con los parametros requeridos en la tarea"
    numero_hilos=16
    tamano_muestra=5000000
    
else
    mensaje=$1  
    tamano_muestra=$2
    numero_hilos=$3
fi


#----------------------------------------------------------------------------
#------------------------ Informacion de ejecucion --------------------------
#----------------------------------------------------------------------------
# El ID  de la corrida es el time stamp
time_stamp=`date +"%d_%m_%Y_%H%M%S"`   
archivo_resultados="r_$time_stamp"

#Se imprime en pantalla la informacion pertinente
echo "▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ Iniciando ejecuccion ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒"
echo " > Bitacora: $mensaje"
echo " > ID de los resultados: $time_stamp"
echo -e "\t•Tamano muesstra: '$tamano_muestra'"
echo -e "\t•Numero de hilos: '$numero_hilos'"
echo -e "\t•Resultados en: '$archivo_resultados'"



#----------------------------------------------------------------------------
#------------------------ Compilacion de archivos ---------------------------
#----------------------------------------------------------------------------
HORA=`date +"%H:%M"`
echo -e "\v======================= Compilando archivos :: $HORA ======================="

echo "_______________ Compilando el archivo secuencial _______________"
mpicc ./secuencial.c -o ejecucion_secuencial.exe
echo -e "----------------------------------------------------------------\n"

echo "________________ Compilando el archivo paralelo ________________"
mpicc ./paralelo.c -o ejecucion_paralela.exe -lm
echo -e "----------------------------------------------------------------\n"


#----------------------------------------------------------------------------
#-------------------------- Ejecucion algoritmos ----------------------------
#----------------------------------------------------------------------------
echo -e "\v > Inicia la corrida de ejecutables"

#------------------------------- Secuencial ---------------------------------
HORA=`date +"%H:%M"`
echo "--------------- corrida secuencial :: $HORA ---------------"
./ejecucion_secuencial.exe $tamano_muestra >> $archivo_resultados

#-------------------------------- Paralelos ---------------------------------
for i in $(seq 1 $numero_hilos)
do
    HORA=`date +"%H:%M"`
    echo "~~~~~~~~~~~~~ Ejecucion con $i hilos :: $HORA ~~~~~~~~~~~~~"
    mpirun -np=$i ./ejecucion_paralela.exe $tamano_muestra >> $archivo_resultados
done


#----------------------------------------------------------------------------
#----------------------------- Fin del archivo ------------------------------
#----------------------------------------------------------------------------
echo -e "░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ Ejecuccion terminada ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░"