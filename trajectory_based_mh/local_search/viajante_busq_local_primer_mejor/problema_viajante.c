#include <stdio.h>
#include <stdlib.h>
#include "problema_viajante.h"

unsigned int n_ciudades = 0;
DISTANCIAS d = NULL;
SOLUCION sol_act = NULL;

int inicializa_datos(unsigned int n, char* archivo_datos) {
    int exito = 0;

    if(n > 1 && archivo_datos != NULL) {
        n_ciudades = n;

        FILE *fp = fopen(archivo_datos, "r");
        if(fp != NULL) {
            d = (unsigned int**)calloc(n_ciudades - 1, sizeof(unsigned int*));
            if(d != NULL) {
                //matriz de distancias
                int i = 0, j = 0;
                int read = 1;
                for(; i < n_ciudades - 1 && read; i++) {
                    d[i] = (unsigned int*)calloc(i + 1, sizeof(unsigned int)); //no hace falta reservar n posiciones en todas las filas
                    if(d[i] == NULL) {
                        elimina_datos();
                        break;
                    }
                    for(j = 0; j < i + 1; j++) {
                        read = fscanf(fp, "%u", &d[i][j]);
                        if(read < 1) {
                            elimina_datos();
                            break;
                        }
                    }
                }

                if(d != NULL) {
                    sol_act = (unsigned int*)calloc(n_ciudades - 1, sizeof(unsigned int));
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
        int i = 0;
        for(; i < n_ciudades - 1; i++) {
            if(d[i] != NULL) {
                free(d[i]);
                d[i] = NULL;
            }
        }
        free(d);
        d = NULL;
    }
    if(sol_act != NULL) {
        free(sol_act);
        sol_act = NULL;
    }
    n_ciudades = 0;
}

unsigned int objetivo(SOLUCION sol_act) {
    unsigned int distancia = 0;

    if(sol_act != NULL) {
        unsigned int ciudad_origen = 0;
        unsigned int ciudad_destino = sol_act[0];
        distancia = d[ciudad_destino - 1][0]; //ciudad de partida -> primera ciudad del recorrido

        int i = 1;
        for(; i < n_ciudades - 1; i++) {
            ciudad_origen = sol_act[i - 1];
            ciudad_destino = sol_act[i];
            distancia += (ciudad_destino >= ciudad_origen) ? d[ciudad_destino - 1][ciudad_origen] : d[ciudad_origen - 1][ciudad_destino];
        }
        ciudad_origen = sol_act[n_ciudades - 2];
        ciudad_destino = 0;
        distancia += d[ciudad_origen - 1][ciudad_destino]; //ultima ciudad del recorrido -> ciudad de partida
    }

    return distancia;
}
