'''
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
direccion_resultados = eg.fileopenbox(msg="Elije el archivo con los tiempos",
                         title="Control: fileopenbox",
                         default='//sshfs/ppd@148.225.111.150/juventino_velasquez_ocotillo/')
direccion_salida = direccion_resultados[:-16]



#----------------------------------------------------------------------------
#------------------------ Se prepara la informacion  ------------------------
#----------------------------------------------------------------------------
#Datos en crudo
tiempos_ejecucion = pd.read_csv(direccion_resultados , sep=',',  names=['nh','tiempo'])

#Datos para tiempos de ejecuccion
tiempo_secuencial = tiempos_ejecucion['tiempo'][0]
tiempo_paralelo = tiempos_ejecucion['tiempo'][1:]
labels_paralelo = tiempos_ejecucion['nh'][1:]

#Datos para el speedup
tiempos_speedup = []
for tiempo in tiempo_paralelo:
    tiempos_speedup.append(tiempo_secuencial /tiempo)
labels_speedup = labels_paralelo


#----------------------------------------------------------------------------
#------------------------- Se grafican los tiempos --------------------------
#----------------------------------------------------------------------------
with plt.style.context('seaborn-darkgrid'):
    diagrama_tiempos , grafica_tiempos = plt.subplots(1)
    grafica_tiempos.plot(1,tiempo_secuencial,
                  color='r',
                  marker='*',
                  markersize=15, 
                  label = 'Tiempo secuencial')
    grafica_tiempos.plot(labels_paralelo ,tiempo_paralelo ,
                  marker='.',
                  label='Variacion del tiempo usando hilos',
                  linestyle='dashed')
    grafica_tiempos.set_xlabel("Número de hilos")
    grafica_tiempos.set_ylabel("Tiempo")
    grafica_tiempos.set_title(titulo+'\ntiempo')
    grafica_tiempos.legend()


#----------------------------------------------------------------------------
#-------------------------- Se grafica el speedup ---------------------------
#----------------------------------------------------------------------------
with plt.style.context('seaborn-darkgrid'):
    diagrama_speedup, grafica_speedup = plt.subplots(1)
    grafica_speedup.plot(labels_speedup ,tiempos_speedup ,
                  label='Speedup segun el numero de hilos')
    grafica_speedup.set_xlabel("Número de hilos")
    grafica_speedup.set_ylabel("SpeedUp")
    grafica_speedup.set_title(titulo+'\nspeedup')
    grafica_speedup.legend()

#----------------------------------------------------------------------------
#------------------------- Se guardan las graficas --------------------------
#----------------------------------------------------------------------------
direccion_grafica_resultados = direccion_salida+'\\resultado_tiempo'
direccion_grafica_speedup = direccion_salida+'\\resultado_speedup'
print(" > Direccion de la grafica con los tiempos: ")
print("\t",direccion_grafica_resultados)
print(" > Direccion de la grafica con el speedup: ")
print("\t",direccion_grafica_speedup)
diagrama_tiempos.savefig(direccion_grafica_resultados)
diagrama_speedup.savefig(direccion_grafica_speedup)