import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import sys
import easygui as eg



#if(sys.argv[1] != None):
    


#----------------------------------------------------------------------------
#-------------------- Se buscan los archivo con los datos ---------------------
#----------------------------------------------------------------------------
opcion = "s"
direcciones_datos = []
nombre_datos = []
etiquetas_datos = []

print("Porfavor eliga el archivo con los datos secuenciales ...")
archivos_datos_secuencial = eg.fileopenbox(
    msg="Elije el archivo con los tiempos",
    title="Control: fileopenbox",
    default='//sshfs/ppd@148.225.111.150/juventino_velasquez_ocotillo/')
 

while(opcion == "s" ):
    print("Porfavor este atento de la ventana emergente ...")
    direccion = eg.fileopenbox(
        msg="Elije el archivo con los tiempos",
        title="Control: fileopenbox",
        default='//sshfs/ppd@148.225.111.150/juventino_velasquez_ocotillo/')
    direcciones_datos.append(direccion)
    descripcion_datos = input("Descripcion con la que los datos aparecen en la grafica: ")
    etiquetas_datos.append(descripcion_datos)
    opcion = input("Quieres añadir más archivos? (s/n) : ")

#----------------------------------------------------------------------------
#------------------------ Se prepara la informacion  ------------------------
#----------------------------------------------------------------------------
tiempos_paralelos = []
tiempos_speedup = []
labels_paralelos = []
etiquetas_bien_ordenadas = []

tiempo_ejecucion_secuencial = pd.read_csv(archivos_datos_secuencial , sep=',',  names=['nh','tiempo','aux'])
tiempo_secuencial = tiempo_ejecucion_secuencial['tiempo'][0]

while(len(direcciones_datos) != 0):
    
    direccion_resultados = direcciones_datos.pop()
    
    
    #Datos en crudo
    tiempos_ejecucion = pd.read_csv(direccion_resultados , sep=',',  names=['nh','tiempo','aux'])
    etiquetas_bien_ordenadas.append(etiquetas_datos.pop())

    #Datos para tiempos de ejecuccion
    tiempo_paralelo_local = tiempos_ejecucion['tiempo'][:]
    tiempos_paralelos.append(tiempo_paralelo_local)
    labels_paralelos.append(tiempos_ejecucion['nh'][:])
    
    #Datos para el speedup
    tiempo_speedup = []
    for tiempo in tiempo_paralelo_local:
        tiempo_speedup.append(tiempo_secuencial / tiempo)
    
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
        
        grafica_tiempos.plot(1,tiempo_secuencial,
                    color='r',
                    marker='*',
                    markersize=15, 
                    label = 'Tiempo secuencial')

        while len(tiempos_paralelos) != 0  :
#----------------------------------------------------------------------------
#-------------------------- Se grafica el tiempo ---------------------------
#----------------------------------------------------------------------------
            tiempo_paralelo = tiempos_paralelos.pop()
            labels_paralelo = labels_paralelos.pop() 
            labels_speedup = labels_paralelo
            tiempo_speedup = tiempos_speedup.pop() 
            etiqueta = etiquetas_bien_ordenadas.pop() 

            grafica_tiempos.plot(labels_paralelo ,tiempo_paralelo ,
                        marker='.',
                        label=etiqueta,
                        linestyle='dashed')
            grafica_tiempos.set_xlabel("Número de hilos")
            grafica_tiempos.set_ylabel("Tiempo")
            grafica_tiempos.set_title(titulo_tiempos)
            grafica_tiempos.legend()

#----------------------------------------------------------------------------
#-------------------------- Se grafica el speedup ---------------------------
#----------------------------------------------------------------------------
            grafica_speedup.plot(labels_speedup ,tiempo_speedup ,
                        label=etiqueta)
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