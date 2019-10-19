#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "lista_tabu.h"
#include "problema_viajante.h"

#define ITE_REINICIALIZACION 100
#define ITE_PARADA 10000

typedef unsigned char** PERMUTACIONES; //matriz binaria que marca las permutaciones generadas (triangular inferior)

PERMUTACIONES p;
lista_tabu lt = NULL;
SOLUCION sol_act = NULL, s_prima = NULL, mejor_vecino = NULL;
int siguiente_vecino_i = 0, siguiente_vecino_j = 0;
unsigned int ciudad = -1;
unsigned char quedan_vecinos = (unsigned char) 0;
unsigned int c_vecino = -1, c_mejor_vecino = -1;
INSERCION insercion_prima, insercion_mejor_vecino;
unsigned int reinicios = 0;

void inicializa_permutaciones() {
    int i = 0, j = 0;

    p = (unsigned char **)calloc(n_ciudades - 2, sizeof(unsigned char*));
    for(; i < n_ciudades - 2; i++) {
        p[i] = (unsigned char *)calloc(i + 1, sizeof(unsigned char));
        for(j = 0; j < i + 1; j++) {
            p[i][j] = (unsigned char) 0; //cero
         }
    }
}

void reinicia_permutaciones() {
    int i = 0, j = 0;

    for(; i < n_ciudades - 2; i++) {
        for(j = 0; j < i + 1; j++) {
            p[i][j] = (unsigned char) 0; //cero
        }
    }
}

void elimina_permutaciones() {

    if(p != NULL) {
        int i = 0;
        for(; i < n_ciudades - 2; i++) {
            if(p[i] != NULL) {
                free(p[i]);
                p[i] = NULL;
            }
        }
        free(p);
        p = NULL;
    }

}

//BUSQUEDA TABU
void genera_solucion_inicial() {
    int i = 0, j = 0;

    for(; i < n_ciudades - 1; i++) {
        solucion[i] = 1 + floor(drand48() * (n_ciudades - 1));

        for(j = 0; j < i; j++) {
            if(solucion[i] == solucion[j]) {
                int repetido = 0;
                do {
                    repetido = 0;
                    solucion[i] = solucion[i] % (n_ciudades - 1) + 1;
                    for(j = 0; j < i; j++) {
                        if(solucion[i] == solucion[j]) {
                            repetido = 1;
                            break;
                        }
                    }
                } while(repetido);
                break;
            }
        }
    }
}

int cumple_crit_asp(SOLUCION s) {
    //criterio de aspiracion
    if(objetivo(s) < objetivo(solucion)) {
        return 1;
    }
    else {
        return 0;
    }
}

void insercion(int indice_ciudad, int indice_insercion) {
    int i, j;
    unsigned int valor_insercion;

    valor_insercion = sol_act[indice_ciudad];
    if(indice_insercion < indice_ciudad) {
        for(i = 0; i < indice_insercion; i++) {
            s_prima[i] = sol_act[i];
        }
        s_prima[indice_insercion] = valor_insercion;
        for(i = indice_insercion, j = indice_insercion + 1; i < indice_ciudad; i++, j++) {
            s_prima[j] = sol_act[i];
        }
        for(i = indice_ciudad + 1; i < n_ciudades - 1; i++) {
            s_prima[i] = sol_act[i];
        }
    }
    else if(indice_insercion > indice_ciudad) {
        for(i = 0; i < indice_ciudad; i++) {
            s_prima[i] = sol_act[i];
        }
        for(i = indice_ciudad + 1, j = indice_ciudad; i <= indice_insercion; i++, j++) {
            s_prima[j] = sol_act[i];
        }
        s_prima[indice_insercion] = valor_insercion;
        for(i = indice_insercion + 1; i < n_ciudades - 1; i++) {
            s_prima[i] = sol_act[i];
        }
    }

    c_vecino = objetivo(s_prima);
}

void genera_vecino() {
    int i = 0, j = 0;
    
    //asignacion del nuevo vecino a s_prima
    insercion(insercion_prima.indice_ciudad, insercion_prima.indice_ins);
    if(cumple_crit_asp(s_prima) || !es_tabu(lt, insercion_prima)) { //tabu
        c_vecino = objetivo(s_prima);
    }
    else {
        c_vecino = 0;
    }

    p[i][j] = (unsigned char) 1;
    siguiente_vecino_j = (j + 1) % (i + 1);
    siguiente_vecino_i = (siguiente_vecino_j == 0)? i + 1 : i;
}



unsigned int busq_tabu() { //devuelve la solucion alcanzada, o 0 en caso de error
    unsigned int c_solucion = 0;

    unsigned int indice_ciudad = -1;
    unsigned int indice_insercion = -1, indice_mejor_ins = -1;
    p = NULL;
    inicializa_permutaciones();
    quedan_vecinos = (unsigned char) 1;
    lt = NULL;
    crear_lista_tabu(&lt, n_ciudades);
    sol_act = (unsigned int *)calloc(n_ciudades - 1, sizeof(unsigned int));
    s_prima = (unsigned int *)calloc(n_ciudades - 1, sizeof(unsigned int));
    mejor_vecino = (unsigned int *)calloc(n_ciudades - 1, sizeof(unsigned int));
    c_vecino = -1;
    c_mejor_vecino = -1;
    reinicios = 0;
    if(p != NULL && lt != NULL && sol_act != NULL && mejor_vecino != NULL && s_prima != NULL) {
        int i = 0, j = 1, ite_sin_mejora = 0, ite_solucion = 0;

        srand48((unsigned)time(NULL));
        genera_solucion_inicial();
        c_solucion = objetivo(solucion);
        printf("RECORRIDO INICIAL\n");
        printf("\tRECORRIDO:");
        for(i = 0; i < n_ciudades - 1; i++) {
            printf(" %u", solucion[i]);
        }
        printf(" \n");
        printf("\tCOSTE (km): %u\n", c_solucion);

        for(i = 0; i < n_ciudades - 1; i++) {
            sol_act[i] = solucion[i]; //primera solucion actual: la solucion inicial
        }

        do {
            printf("\nITERACION: %d\n", j);
            
            indice_ciudad = floor(drand48() * (n_ciudades - 1));
            insercion_prima.indice_ciudad = indice_ciudad;
            ciudad = sol_act[indice_ciudad];
            printf("\tINDICE CIUDAD: %d\n", indice_ciudad);
            printf("\tCIUDAD: %u\n", ciudad);
            
            //inicialmente, el mejor vecino es el primero
            c_vecino = 0;
            indice_insercion = (indice_ciudad + 1) % (n_ciudades - 1);
            insercion_prima.indice_ins = indice_insercion;
            genera_vecino();
            c_mejor_vecino = c_vecino;
            indice_mejor_ins = indice_insercion;
            insercion_prima.indice_ins = indice_mejor_ins;
            memcpy(&insercion_mejor_vecino, &insercion_prima, sizeof(INSERCION));
            indice_insercion = (indice_insercion + 1) % (n_ciudades - 1);
            //se busca el mejor de todos los que no son tabu
            do {
                genera_vecino();
                if(c_vecino != 0 && (c_vecino < c_mejor_vecino || (c_vecino == c_mejor_vecino && indice_insercion < indice_mejor_ins))) {
                    for(i = 0; i < n_ciudades - 1; i++) {
                        mejor_vecino[i] = s_prima[i];
                    }
                    c_mejor_vecino = c_vecino;
                    memcpy(&insercion_mejor_vecino, &insercion_prima, sizeof(INSERCION));
                }
                
                 indice_insercion = (indice_insercion + 1) % (n_ciudades - 1);
                 insercion_prima.indice_ins = indice_insercion;
            } while(indice_insercion != indice_ciudad);
            printf("\tINSERCION: (%u, %u)\n", insercion_mejor_vecino.indice_ciudad, insercion_mejor_vecino.indice_ins);
            //se asigna el mejor como la solucion actual y se inserta el movimiento en la lista tabu
            printf("\tRECORRIDO:");
            for(i = 0; i < n_ciudades - 1; i++) {
                sol_act[i] = mejor_vecino[i];
                printf(" %u", sol_act[i]);
            }
            printf(" \n");
            printf("\tCOSTE (km): %u\n", objetivo(sol_act));
            insertar_tabu(lt, insercion_mejor_vecino);

            if(c_mejor_vecino < c_solucion) { //el mejor vecino es optimo "global"
                for(i = 0; i < n_ciudades - 1; i++) {
                    solucion[i] = mejor_vecino[i];
                }
                c_solucion = objetivo(solucion);
                ite_solucion = j;

                ite_sin_mejora = 0;
            }
            else { //no se ha mejorado la solucion optima
                ite_sin_mejora++;
            }
            printf("\tITERACIONES SIN MEJORA: %d\n", ite_sin_mejora);
            imprimir_lista_tabu(lt);

            if(ite_sin_mejora == ITE_REINICIALIZACION) {
                //reinicializacion
                for(i = 0; i < n_ciudades - 1; i++) {
                    sol_act[i] = solucion[i];
                }
                vaciar_lista_tabu(lt);

                ite_sin_mejora = 0;
                reinicios++;
                printf("\n***************\nREINICIO: %u\n***************\n", reinicios);
            }

            quedan_vecinos = (unsigned char) 1;
            siguiente_vecino_i = 0;
            siguiente_vecino_j = 0;
            j++;
        } while(j <= ITE_PARADA);

        printf("\n\nMEJOR SOLUCION: \n");
        printf("\tRECORRIDO:");
        for(i = 0; i < n_ciudades - 1; i++) {
            printf(" %u", solucion[i]);
        }
        printf(" \n");
        printf("\tCOSTE (km): %u\n", c_solucion);
        printf("\tITERACION: %d\n", ite_solucion);
    }

    if(mejor_vecino != NULL) {
        free(mejor_vecino);
        mejor_vecino = NULL;
    }
    if(s_prima != NULL) {
        free(s_prima);
        s_prima = NULL;
    }
    if(sol_act != NULL) {
        free(sol_act);
        sol_act = NULL;
    }
    destruir_lista_tabu(&lt);
    elimina_permutaciones();

    return c_solucion;
}


//BUSQUEDA TABU A PARTIR DE ARCHIVO
FILE *archivo_aleatorios = NULL;

void genera_solucion_inicial_archivo() {
    int i = 0, j = 0;
    double r = 0.0;

    for(; i < n_ciudades - 1; i++) {
        fscanf(archivo_aleatorios, "%lf\r\n", &r);
        solucion[i] = 1 + floor(r * (n_ciudades - 1));

        for(j = 0; j < i; j++) {
            if(solucion[i] == solucion[j]) {
                int repetido = 0;
                do {
                    repetido = 0;
                    solucion[i] = solucion[i] % (n_ciudades - 1) + 1;
                    for(j = 0; j < i; j++) {
                        if(solucion[i] == solucion[j]) {
                            repetido = 1;
                            break;
                        }
                    }
                } while(repetido);
                break;
            }
        }
    }
}

unsigned int busq_tabu_archivo(char* traza) { //devuelve la solucion alcanzada, o 0 en caso de error
    unsigned int c_solucion = 0;

    archivo_aleatorios = fopen(traza, "r");
    if(archivo_aleatorios != NULL) {
        p = NULL;
        inicializa_permutaciones();
        quedan_vecinos = (unsigned char) 1;
        lt = NULL;
        crear_lista_tabu(&lt, n_ciudades);
        sol_act = (unsigned int *)calloc(n_ciudades - 1, sizeof(unsigned int));
        s_prima = (unsigned int *)calloc(n_ciudades - 1, sizeof(unsigned int));
        mejor_vecino = (unsigned int *)calloc(n_ciudades - 1, sizeof(unsigned int));
        c_vecino = -1;
        c_mejor_vecino = -1;
        reinicios = 0;
        if(p != NULL && lt != NULL && sol_act != NULL && mejor_vecino != NULL && s_prima != NULL) {
            int i = 0, j = 1, ite_sin_mejora = 0, ite_solucion = 0;

            srand48((unsigned)time(NULL));
            genera_solucion_inicial_archivo();
            fclose(archivo_aleatorios);
            archivo_aleatorios = NULL;
            c_solucion = objetivo(solucion);
            printf("RECORRIDO INICIAL\n");
            printf("\tRECORRIDO:");
            for(i = 0; i < n_ciudades - 1; i++) {
                printf(" %u", solucion[i]);
            }
            printf(" \n");
            printf("\tCOSTE (km): %u\n", c_solucion);

            for(i = 0; i < n_ciudades - 1; i++) {
                sol_act[i] = solucion[i]; //primera solucion actual: la solucion inicial
            }

            do {
                printf("\nITERACION: %d\n", j);
                //inicialmente, el mejor vecino es el primero
                c_vecino = 0;
                do {
                    genera_vecino();
                } while(c_vecino == 0);
                for(i = 0; i < n_ciudades - 1; i++) {
                    mejor_vecino[i] = s_prima[i];
                }
                c_mejor_vecino = c_vecino;
                memcpy(&insercion_mejor_vecino, &insercion, sizeof(INTERCAMBIO));
                //se busca el mejor de todos los que no son tabu
                do {
                    genera_vecino();
                    if(c_vecino != 0 && c_vecino < c_mejor_vecino) {
                        for(i = 0; i < n_ciudades - 1; i++) {
                            mejor_vecino[i] = s_prima[i];
                        }
                        c_mejor_vecino = c_vecino;
                        memcpy(&insercion_mejor_vecino, &insercion, sizeof(INTERCAMBIO));
                    }
                } while(quedan_vecinos);
                printf("\tINTERCAMBIO: (%u, %u)\n", insercion_mejor_vecino.indice_ciudad, insercion_mejor_vecino.indice_ins);

                //se asigna el mejor como la solucion actual y se inserta el movimiento en la lista tabu
                printf("\tRECORRIDO:");
                for(i = 0; i < n_ciudades - 1; i++) {
                    sol_act[i] = mejor_vecino[i];
                    printf(" %u", sol_act[i]);
                }
                printf(" \n");
                printf("\tCOSTE (km): %u\n", objetivo(sol_act));
                insertar_tabu(lt, insercion_mejor_vecino);


                if(c_mejor_vecino < c_solucion) { //el mejor vecino es optimo "global"
                    for(i = 0; i < n_ciudades - 1; i++) {
                        solucion[i] = mejor_vecino[i];
                    }
                    c_solucion = objetivo(solucion);
                    ite_solucion = j;

                    ite_sin_mejora = 0;
                }
                else { //no se ha mejorado la solucion optima
                    ite_sin_mejora++;
                }
                printf("\tITERACIONES SIN MEJORA: %d\n", ite_sin_mejora);
                imprimir_lista_tabu(lt);

                if(ite_sin_mejora == ITE_REINICIALIZACION) {
                    //reinicializacion
                    for(i = 0; i < n_ciudades - 1; i++) {
                        sol_act[i] = solucion[i];
                    }
                    vaciar_lista_tabu(lt);

                    ite_sin_mejora = 0;
                    reinicios++;
                    printf("\n***************\nREINICIO: %u\n***************\n", reinicios);
                }

                reinicia_permutaciones();
                quedan_vecinos = (unsigned char) 1;
                siguiente_vecino_i = 0;
                siguiente_vecino_j = 0;
                j++;
            } while(j <= ITE_PARADA);

            printf("\n\nMEJOR SOLUCION: \n");
            printf("\tRECORRIDO:");
            for(i = 0; i < n_ciudades - 1; i++) {
                printf(" %u", solucion[i]);
            }
            printf(" \n");
            printf("\tCOSTE (km): %u\n", c_solucion);
            printf("\tITERACION: %d\n", ite_solucion);
        }

        if(mejor_vecino != NULL) {
            free(mejor_vecino);
            mejor_vecino = NULL;
        }
        if(s_prima != NULL) {
            free(s_prima);
            s_prima = NULL;
        }
        if(sol_act != NULL) {
            free(sol_act);
            sol_act = NULL;
        }
        destruir_lista_tabu(&lt);
        elimina_permutaciones();
    }


    return c_solucion;
}
