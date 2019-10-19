#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mh_busq_tabu.h"
#include "problema_viajante.h"

int main(int argc, char** argv) {
    unsigned int resultado;
    int inicio;

    if(argc >= 5 && strncmp(argv[1], "-f", strlen("-f")) == 0) {
        inicio = inicializa_datos(atoi(argv[3]), argv[4]);
        if(!inicio) {
            printf("Error en búsqueda tabú desde archivo: no se pudieron cargar los datos del problema correctamente.\n");
            exit(1);
        }

        resultado = busq_tabu_archivo(argv[2]);
    }
    else if(argc == 4 && strncmp(argv[1], "-a", strlen("-a")) == 0) {
        inicio = inicializa_datos(atoi(argv[2]), argv[3]);
        if(!inicio) {
            printf("Error en búsqueda tabú aleatoria: no se pudieron cargar los datos del problema correctamente.\n");
            exit(1);
        }

        resultado = busq_tabu();
    }
    else {
        printf("Error: parámetros no especificados correctamente.\n");
        printf("Formato para búsqueda tabú aleatoria: %s -a [n ciudades] [archivo de distancias]\n", argv[0]);
        printf("Formato para búsqueda tabú desde archivo: %s -f [archivo de aleatorios] [n ciudades] [archivo de distancias]\n", argv[0]);
        exit(1);
    }

    if(!resultado) {
        printf("Error al aplicar la metaheurística. No se pudo calcular la solución.\n\n");
    }
    else {
        /*
        printf("Distancia óptima: %u\n", resultado);
        printf("Recorrido óptimo: ");
        int i = 0;
        for(; i < n_ciudades - 2; i++) {
            printf("%u-", solucion[i]);
        }
        printf("%u\n\n", solucion[i]);
        */
    }

    elimina_datos();

    exit(0);
}
