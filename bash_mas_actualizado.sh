#!/bin/bash
# Autor: Luis Juventino Velasquez Hidalgo
# Fecha: 2/19/2023
#   Archivo para mostrar la diferencia entre los tiempos del programa
#   Solo funciona en ocotillo, deberia poner tambien el del choya

# Para choya?
#SBATCH --nodes=5           
#SBATCH --job-name=OpenMP    
#SBATCH --ntasks-per-node=4
#SBATCH --time=1:00:00

#Ocotillo
#SBATCH --nodes=1
#SBATCH --job-name=MPI
#SBATCH --ntasks=16
#SBATCH --time=1:00:00
#SBATCH --partition=general
#SBATCH --constraint=broadwell
#SBATCH --mem-per-cpu=3200


# module load intel/oneAPI-2021 #Usar solo si usas MPI
# export I_MPI_HYDRA_BOOTSTRAP=ssh
#----------------------------------------------------------------------------
#------------------------ Variables de entorno ------------------------------
#----------------------------------------------------------------------------
if [[ $# != 3 ]]
then
    mensaje="Ejecucion con los parametros requeridos en la tarea"
    numero_hilos=16
    tamano_muestra=5000000
    
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
#g++ ./secuencialc.cpp -fopenmp -o ejecucion_secuencial.exe # <- OpenMP
#mpicc ./secuencial.c -o ejecucion_secuencial.exe #<- Usando mpi
echo -e "\n\n======================= Compilando archivos :: $HORA ======================="


echo "_______________ Compilando el archivo secuencial _______________"
mpicc ./secuencial.c -o ejecucion_secuencial.exe
echo -e "----------------------------------------------------------------\n"

echo "________________ Compilando el archivo paralelo ________________"
mpicc ./paralelo.c -o ejecucion_paralela.exe
echo -e "----------------------------------------------------------------\n"

# Por si se necesita generar.
#echo "________________ Compilando el archivo de datos ________________"
#g++ generar_numeros.c -o ejecutar_generar.exe
#echo -e "----------------------------------------------------------------\n"


#----------------------------------------------------------------------------
#-------------------------- Ejecucion algoritmos ----------------------------
#----------------------------------------------------------------------------
#   mpirun -np=$i ./ejecucion_paralela.exe $tamano_muestra >> $archivo_resultados #Si se usa MPI
#   ./ejecucion_paralela.exe $tamano_muestra #hilos >> $archivo_resultados #Si se usa OpenMP
#   Para usar OpenMP, se pasa la cantidad de nodos POR PARAMETRO :) 
echo -e "\n\n======================= Corriendo ejecutables :: $HORA ======================="


# Por si se necesita el archivo de datos
#echo -e " > Inicia el archivo generador de datos"
#echo "----------------- Generando datos :: $HORA ----------------"
#./ejecutar_generar.exe 'datos' $tamano_muestra >> $archivo_resultados

#------------------------------- Secuencial ---------------------------------
echo -e "\n > Inicia la corrida secuencial"
HORA=`date +"%H:%M:%S"`
echo "--------------- corrida secuencial :: $HORA ---------------"
./ejecucion_secuencial.exe $tamano_muestra >> $archivo_resultados


#-------------------------------- Paralelos ---------------------------------
echo -e "\n > Inician las corridas paralelas"
for i in $(seq 1 $numero_hilos)
do
    echo "~~~~~~~~~~~~~ Ejecucion con $i hilos :: $HORA ~~~~~~~~~~~~~~"
    HORA=`date +"%H:%M:%S"`
    mpirun -np=$i ./ejecucion_paralela.exe $tamano_muestra >> $archivo_resultados
done


#----------------------------------------------------------------------------
#----------------------------- Fin del archivo ------------------------------
#----------------------------------------------------------------------------
echo -e "░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ Ejecuccion terminada ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░"
