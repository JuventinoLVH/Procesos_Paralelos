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
#   Primer parametro: Bitacora que se imprime en el slurm
#   Segundo parametro:  Tamaño de muestra
#   Tercer parametro: Numero de hilos a usar 

#----------------------------------------------------------------------------
#------------------------ Variables de entorno ------------------------------
#----------------------------------------------------------------------------
if [[ $# != 3 ]]
then
    mensaje="Ejecucion con los parametros requeridos en la tarea"
    numero_hilos=16
    tamano_muestra=10000000
    
else
#Primero mensaje, despues el tamano y por ultimo, la cantidad de hilos
    mensaje=$1  
    tamano_muestra=$2
    numero_hilos=$3
fi

#----------------------------------------------------------------------------
#------------------------ Informacion de ejecucion --------------------------
#----------------------------------------------------------------------------
# El ID  de la corrida es el time stamp
time_stamp=`date +"%d.%m.%Y-%H_%M_%S"`   
archivo_resultados="r_$time_stamp.bin"

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
HORA=`date +"%H:%M:%S"`
echo -e "\v======================= Compilando archivos :: $HORA ======================="


echo "_______________ Compilando el archivo secuencial _______________"
g++ ./secuencial.c -fopenmp -o ejecucion_secuencial.exe 
echo -e "----------------------------------------------------------------\n"

echo "________________ Compilando el archivo paralelo ________________"
g++ ./paralelo.c -fopenmp -o ejecucion_paralela.exe
echo -e "----------------------------------------------------------------\n"

echo "________________ Compilando el archivo de datos ________________"
g++ generar_numeros.c -o ejecutar_generar.exe
echo -e "----------------------------------------------------------------\n"



#----------------------------------------------------------------------------
#-------------------------- Ejecucion algoritmos ----------------------------
#----------------------------------------------------------------------------
echo "----------------- Generando datos :: $HORA ----------------"
./ejecutar_generar.exe 'datos.bin' $tamano_muestra >> $archivo_resultados

echo -e "\v > Inicia la corrida de ejecutables"
#------------------------------- Secuencial ---------------------------------
HORA=`date +"%H:%M:%S"`
echo "--------------- corrida secuencial :: $HORA ---------------"
./ejecucion_secuencial.exe 'datos' >> $archivo_resultados

#-------------------------------- Paralelos ---------------------------------
for i in $(seq 1 $numero_hilos)
do
    HORA=`date +"%H:%M:%S"`
    echo "~~~~~~~~~~~~~ Ejecucion con $i hilos :: $HORA ~~~~~~~~~~~~~"
    ./ejecucion_paralela.exe $i 'datos' >> $archivo_resultados
done


#----------------------------------------------------------------------------
#----------------------------- Fin del archivo ------------------------------
#----------------------------------------------------------------------------
echo -e "░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ Ejecuccion terminada ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░"
