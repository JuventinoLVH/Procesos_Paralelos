''' 
    NOTA: La primera dereccion debe de ser la de OpenMP y la segunda de MPI
    Archivo para crear graficas, es solo para ejecutarse en computadora personal.
# param 1: Titulo de la grafica
'''
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import sys
import easygui as eg

titulo = sys.argv[1]

#----------------------------------------------------------------------------
#-------------------- Se busca el archivo con los datos ---------------------
#----------------------------------------------------------------------------
print("Porfavor este atento de la ventana emergente ...")
primer_direccion_resultados = eg.fileopenbox(msg="Elije el primer archivo con los tiempos",
                         title="Control: fileopenbox",
                         default='//sshfs/ppd@148.225.111.150/juventino_velasquez_ocotillo/')
primera_direccion_salida = primer_direccion_resultados[:-16]

print("Exito al seleccionar el archivo de OpenMP, ahora sigue el de MPI...")
segundo_direccion_resultados = eg.fileopenbox(msg="Elije el segundo archivo con los tiempos",
                         title="Control: fileopenbox",
                         default='//sshfs/ppd@148.225.111.150/juventino_velasquez_ocotillo/')
segunda_direccion_salida = segundo_direccion_resultados[:-16]

#----------------------------------------------------------------------------
#------------------------ Se prepara la informacion  ------------------------
#----------------------------------------------------------------------------
#Datos en crudo
tiempos_primer_archivo = pd.read_csv(primer_direccion_resultados , sep=',',  names=['nh','tiempo','aux'])
tiempos_segundo_archivo = pd.read_csv(segundo_direccion_resultados , sep=',',  names=['nh','tiempo','aux'])


#Datos para tiempos de ejecuccion
primer_tiempo_secuencial = tiempos_primer_archivo['tiempo'][0]
primer_tiempo_paralelo = tiempos_primer_archivo['tiempo'][1:]
primer_labels_paralelo = tiempos_primer_archivo['nh'][1:]

segundo_tiempo_secuencial = tiempos_segundo_archivo['tiempo'][0]
segundo_tiempo_paralelo = tiempos_segundo_archivo['tiempo'][1:]
segundo_labels_paralelo = tiempos_segundo_archivo['nh'][1:]

#Datos para el speedup
primer_tiempos_speedup = []
for tiempo in primer_tiempo_paralelo:
    primer_tiempos_speedup.append(tiempos_primer_archivo['tiempo'][1]/tiempo)
primer_labels_speedup = primer_labels_paralelo

segundo_tiempos_speedup = []
for tiempo in segundo_tiempo_paralelo:
    segundo_tiempos_speedup.append(tiempos_segundo_archivo['tiempo'][1] /tiempo)
segundo_labels_speedup = segundo_labels_paralelo


#----------------------------------------------------------------------------
#------------------------- Se grafican los tiempos --------------------------
#----------------------------------------------------------------------------
with plt.style.context('seaborn-darkgrid'):
    diagrama_tiempos , grafica_tiempos = plt.subplots(1)
    grafica_tiempos.plot(primer_labels_paralelo ,primer_tiempo_paralelo ,
                  label='Tiempo OpenMP',
                  linestyle=(0, (3, 5, 1, 5, 1, 5)) )
    grafica_tiempos.plot(segundo_labels_paralelo ,segundo_tiempo_paralelo ,
                  label='Tiempo MPI',
                  linestyle= (0, (3, 5, 1, 5)) )
    
    grafica_tiempos.set_xlabel("Número de hilos")
    grafica_tiempos.set_ylabel("Tiempo")
    grafica_tiempos.set_title(titulo+'\ntiempo')
    grafica_tiempos.legend()


#----------------------------------------------------------------------------
#-------------------------- Se grafica el speedup ---------------------------
#----------------------------------------------------------------------------
with plt.style.context('seaborn-darkgrid'):
    diagrama_speedup, grafica_speedup = plt.subplots(1)
    grafica_speedup.plot(primer_labels_speedup ,primer_tiempos_speedup ,linestyle=(0, (3, 5, 1, 5, 1, 5)),
                  label='Speedup OpenMP')
    
    grafica_speedup.plot(segundo_labels_speedup,segundo_tiempos_speedup ,linestyle=(0, (3, 5, 1, 5)),
                  label='Speedup MPI')
    grafica_speedup.set_xlabel("Número de hilos")
    grafica_speedup.set_ylabel("SpeedUp")
    grafica_speedup.set_title(titulo+'\nspeedup')
    grafica_speedup.legend()

#----------------------------------------------------------------------------
#------------------------- Se guardan las graficas --------------------------
#----------------------------------------------------------------------------
direccion_grafica_resultados = '.\\resultado_tiempo'
direccion_grafica_speedup = '.\\resultado_speedup'
print(" > Direccion de la grafica con los tiempos: ")
print("\t",direccion_grafica_resultados)
print(" > Direccion de la grafica con el speedup: ")
print("\t",direccion_grafica_speedup)
diagrama_tiempos.savefig(direccion_grafica_resultados)
diagrama_speedup.savefig(direccion_grafica_speedup)