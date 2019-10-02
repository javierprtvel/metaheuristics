#include <stdio.h>
#include <stdlib.h>
#include "problema_viajante.h"

unsigned int n_ciudades = 0;
DISTANCIAS d = NULL;
SOLUCION solucion = NULL;

int inicializa_datos(unsigned int n, char* archivo_datos) {
    int exito = 0;

    if(n > 1 && archivo_datos != NULL) {
        n_ciudades = n;
        ////printf("Ciudades: %u\n", n_ciudades);

        FILE *fp = fopen(archivo_datos, "r");
        if(fp != NULL) {
            d = (unsigned int**)calloc(n_ciudades - 1, sizeof(unsigned int*));
            if(d != NULL) {
                //matriz de distancias
                int i = 0, j = 0;
                int read = 1;
                ////printf("Distancias:\n");
                for(; i < n_ciudades - 1 && read; i++) {
                    d[i] = (unsigned int*)calloc(i + 1, sizeof(unsigned int)); //no hace falta reservar n posiciones en todas las filas
                    if(d[i] == NULL) {
                        elimina_datos();
                        break;
                    }
                    ////printf("[ ");
                    for(j = 0; j < i + 1; j++) {
                        read = fscanf(fp, "%u", &d[i][j]);
                        ////printf("%u ", d[i][j]);
                        if(read < 1) {
                            elimina_datos();
                            break;
                        }
                    }
                    ////printf("]\n");
                }
                ////printf("\n");

                if(d != NULL) {
                    solucion = (unsigned int*)calloc(n_ciudades - 1, sizeof(unsigned int));
                }
            }

            fclose(fp);
            exito = 1;
        }
    }

    return exito;
}

void elimina_datos() {

    if(d != NULL) {
        //printf("Liberando matriz de distancias...\n");
        int i = 0;
        for(; i < n_ciudades - 1; i++) {
            if(d[i] != NULL) {
                ////printf("Fila %d\n", i + 1);
                free(d[i]);
                d[i] = NULL;
            }
        }
        free(d);
        d = NULL;
    }
    if(solucion != NULL) {
        //printf("Liberando solución...\n");
        free(solucion);
        solucion = NULL;
    }
    n_ciudades = 0;
}

unsigned int objetivo(SOLUCION solucion) {
    unsigned int distancia = 0;

    if(solucion != NULL) {
        unsigned int ciudad_origen = 0;
        unsigned int ciudad_destino = solucion[0];
        distancia = d[ciudad_destino - 1][0]; //ciudad de partida -> primera ciudad del recorrido

        int i = 1;
        for(; i < n_ciudades - 1; i++) {
            ciudad_origen = solucion[i - 1];
            ciudad_destino = solucion[i];
            distancia += (ciudad_destino >= ciudad_origen) ? d[ciudad_destino - 1][ciudad_origen] : d[ciudad_origen - 1][ciudad_destino];
        }
        ciudad_origen = solucion[n_ciudades - 2];
        ciudad_destino = 0;
        distancia += d[ciudad_origen - 1][ciudad_destino]; //ultima ciudad del recorrido -> ciudad de partida
    }

    return distancia;
}
