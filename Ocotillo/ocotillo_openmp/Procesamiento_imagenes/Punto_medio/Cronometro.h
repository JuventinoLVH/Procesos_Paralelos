#ifndef CRONOMETRO_H
#define CRONOMETRO_H

#include <iostream>
#include <ctime> 
#include <iomanip>

class Cronometro
{
    public:    
        //-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
        /* Constructor por default.
        *   > Le asigna el mismo valor a las variables "tiempo_inicio" y tiempo_fin
        *   > Pone la variable corriendo en false
        */
        Cronometro();

        //-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
        /* Empieza a contar el tiempo 
        *   > Cambia el estado de la variable tiempo_inicio y corriendo
        */
        void Iniciar();
        
        //-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
        /* Termina de contar el tiempo
        *   > Cambia el valor de la variable corriendo y tiempo final
        */
        void Parar();

        //-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
        /* Optiene el tiempo que lleva desde que se inicio el cronometro
        *   \return si corriendo es falso, retorna el valor desde tiempo_inicio hasta  
        *               tiempo_fin de lo contrario, devuelve la diferencia entre el  
        *               tiempo actual hasta el tiempo_inicio
        */
        double get_tiempo(); 
    private:
        unsigned tiempo_inicio,tiempo_fin;
        bool corriendo;
};

#endif 