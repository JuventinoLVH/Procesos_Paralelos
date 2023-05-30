import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import sys

path_resultados = sys.argv[1] + "/" + sys.argv[2]
df1 = pd.read_csv(path_resultados , sep=',',  names=['nh','tiempo'])

tiempo_Secuencial = df1['tiempo'][0]
tiempo_paralelo = df1['tiempo'][1:]
labels_paralelo = df1['nh'][1:]

plt.plot(1,   tiempo_Secuencial,color='r',marker='*',markersize=15)
plt.text(1.2, tiempo_Secuencial,'<--- tiempo secuencial',color='r')
plt.plot(labels_paralelo ,tiempo_paralelo ,color='k',marker='.',label='Variacion del tiempo usando hilos',linestyle='dashed')
plt.xlabel("Numero de hilos")
plt.ylabel("Tiempo")
plt.legend()


#Esta echo para funcionar solo con la forma en que yo organizo los archivos 
# Juventino / 'codigo' / lenguaje de implementacion / archivo.bash
codigo = sys.argv[1].split('/')[-2]
titulo = codigo + " muestra = "+sys.argv[3] 
plt.title(titulo)

path_output = sys.argv[1]+"/plot_"+sys.argv[2]
plt.savefig(path_output)