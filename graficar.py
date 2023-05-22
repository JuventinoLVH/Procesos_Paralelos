'''
    Archivo para crear graficas, es solo para ejecutarse en computadora personal.
# param 1: Bandera que dice si se usan los ultimos datos para graficar  
'''

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import sys
import easygui as eg



#if(sys.argv[1] != None):
    


#----------------------------------------------------------------------------
#-------------------- Se busca el archivo con los datos ---------------------
#----------------------------------------------------------------------------
opcion = "s"
direcciones_datos = []

while(opcion == "s" ):
    print("Porfavor este atento de la ventana emergente ...")
    direccion = eg.fileopenbox(
        msg="Elije el archivo con los tiempos",
        title="Control: fileopenbox",
        default='//sshfs/ppd@148.225.111.150/juventino_velasquez_ocotillo/')
    direcciones_datos.append(direccion)
    opcion = input("Quieres añadir más archivos? (s/n) : ")


#----------------------------------------------------------------------------
#------------------------ Se prepara la informacion  ------------------------
#----------------------------------------------------------------------------
tiempos_secuenciales = []
tiempos_paralelos = []
tiempos_speedup = []
labels_paralelos = []

while(len(direcciones_datos) != 0):
    
    direccion_resultados = direcciones_datos.pop()
    
    
    #Datos en crudo
    tiempos_ejecucion = pd.read_csv(direccion_resultados , sep=',',  names=['nh','tiempo','aux'])

    #Datos para tiempos de ejecuccion
    tiempo_secuencial_local = tiempos_ejecucion['tiempo'][0]
    tiempos_secuenciales.append(tiempo_secuencial_local)
    
    tiempo_paralelo_local = tiempos_ejecucion['tiempo'][1:]
    tiempos_paralelos.append(tiempo_paralelo_local)

    labels_paralelos.append(tiempos_ejecucion['nh'][1:])

    #Datos para el speedup
    tiempo_speedup = []
    for tiempo in tiempo_paralelo_local:
        tiempo_speedup.append(tiempo_secuencial_local / tiempo)
    
    tiempos_speedup.append(tiempo_speedup)

#----------------------------------------------------------------------------
#------------------------- Se grafican los tiempos --------------------------
#----------------------------------------------------------------------------

titulo_tiempos = input("Titulo de la grafica de tiempos: ")
titulo_speedup = input("Titulo de la grafica del speedup: ")
with plt.style.context('seaborn-darkgrid'):
    with plt.style.context('seaborn-darkgrid'):
        diagrama_speedup, grafica_speedup = plt.subplots(1)
        diagrama_tiempos , grafica_tiempos = plt.subplots(1)

        while len(tiempos_secuenciales) != 0  :
#----------------------------------------------------------------------------
#-------------------------- Se grafica el tiempo ---------------------------
#----------------------------------------------------------------------------
            tiempo_secuencial = tiempos_secuenciales.pop()
            tiempo_paralelo = tiempos_paralelos.pop()
            labels_paralelo = labels_paralelos.pop() 
            labels_speedup = labels_paralelo
            tiempo_speedup = tiempos_speedup.pop() 

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
            grafica_tiempos.set_title(titulo_tiempos)
            grafica_tiempos.legend()

#----------------------------------------------------------------------------
#-------------------------- Se grafica el speedup ---------------------------
#----------------------------------------------------------------------------
            grafica_speedup.plot(labels_speedup ,tiempo_speedup ,
                        label='Speedup segun el numero de hilos')
            grafica_speedup.set_xlabel("Número de hilos")
            grafica_speedup.set_ylabel("SpeedUp")
            grafica_speedup.set_title(titulo_speedup)
            grafica_speedup.legend()


#----------------------------------------------------------------------------
#------------------------- Se guardan las graficas --------------------------
#----------------------------------------------------------------------------
direccion_grafica_resultados = './resultado_tiempo'
direccion_grafica_speedup = './resultado_speedup'

print(" > Direccion de la grafica con los tiempos: ")
print("\t",direccion_grafica_resultados)
print(" > Direccion de la grafica con el speedup: ")
print("\t",direccion_grafica_speedup)
diagrama_tiempos.savefig(direccion_grafica_resultados)
diagrama_speedup.savefig(direccion_grafica_speedup)