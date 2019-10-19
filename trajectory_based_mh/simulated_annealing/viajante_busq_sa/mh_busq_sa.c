#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "problema_viajante.h"

#define PHI 0.5
#define MU 0.01
#define MAX_VECINOS 80
#define MAX_EXITOS 20
#define ITE_PARADA 10000

SOLUCION sol_act = NULL, s_prima = NULL, mejor_vecino = NULL;
unsigned int c_vecino = -1, c_mejor_vecino = -1;
unsigned int enfriamientos = 0;
int delta = 0;
double temperatura_inicial = 0.0, temperatura = 0.0, exponencial = 0.0, aleatorio = 0.0;
unsigned int candidatas_prob = 0, candidatas_acept = 0;

//BUSQUEDA SA
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

void genera_solucion_candidata() {
    int indice_ciudad, indice_insercion, indice_mejor_ins;
    unsigned int ciudad;
    int i;

    indice_ciudad = floor(drand48() * (n_ciudades - 1));
    ciudad = sol_act[indice_ciudad];
    printf("\tINDICE CIUDAD: %d\n", indice_ciudad);
    printf("\tCIUDAD: %u\n", ciudad);

    //inicialmente, el mejor vecino es el primero
    c_vecino = 0;
    indice_insercion = (indice_ciudad + 1) % (n_ciudades - 1);
    insercion(indice_ciudad, indice_insercion);
    for(i = 0; i < n_ciudades - 1; i++) {
        mejor_vecino[i] = s_prima[i];
    }
    c_mejor_vecino = c_vecino;
    indice_mejor_ins = indice_insercion;
    indice_insercion = (indice_insercion + 1) % (n_ciudades - 1);
    //se busca el mejor de todos
    do {
        insercion(indice_ciudad, indice_insercion);
        if(c_vecino < c_mejor_vecino || (c_vecino == c_mejor_vecino && indice_insercion < indice_mejor_ins)) {
            for(i = 0; i < n_ciudades - 1; i++) {
                mejor_vecino[i] = s_prima[i];
            }
            c_mejor_vecino = c_vecino;
            indice_mejor_ins = indice_insercion;
        }

        indice_insercion = (indice_insercion + 1) % (n_ciudades - 1);
    } while(indice_insercion != indice_ciudad);
    printf("\tINDICE INSERCION: %u\n", indice_mejor_ins);
    printf("\tRECORRIDO:");
    for(i = 0; i < n_ciudades - 1; i++) {
        printf(" %u", mejor_vecino[i]);
    }
    printf(" \n");
    printf("\tFUNCION OBJETIVO (km): %u\n", c_mejor_vecino);
}

void enfriar(int k) {
    temperatura = temperatura_inicial / (1 + k);
    enfriamientos++;
    candidatas_prob = 0;
    candidatas_acept = 0;
    printf("\n============================\nENFRIAMIENTO: %u\n============================\n", enfriamientos);
    printf("TEMPERATURA: %.6lf\n", temperatura);
}

unsigned int busq_sa() { //devuelve la solucion alcanzada, o 0 en caso de error
    unsigned int c_solucion = 0;

    sol_act = (unsigned int *)calloc(n_ciudades - 1, sizeof(unsigned int));
    s_prima = (unsigned int *)calloc(n_ciudades - 1, sizeof(unsigned int));
    mejor_vecino = (unsigned int *)calloc(n_ciudades - 1, sizeof(unsigned int));
    c_vecino = -1;
    c_mejor_vecino = -1;
    enfriamientos = 0;
    candidatas_prob = 0, candidatas_acept = 0;
    if(sol_act != NULL && mejor_vecino != NULL && s_prima != NULL) {
        int i = 0, k = 1, ite_solucion = 0;

        srand48((unsigned)time(NULL));
        genera_solucion_inicial();
        c_solucion = objetivo(solucion);
        printf("SOLUCION INICIAL:\n");
        printf("\tRECORRIDO:");
        for(i = 0; i < n_ciudades - 1; i++) {
            printf(" %u", solucion[i]);
        }
        printf(" \n");
        for(i = 0; i < n_ciudades - 1; i++) { //primera solucion actual: la solucion inicial
            sol_act[i] = solucion[i];
        }
        printf("\tFUNCION OBJETIVO (km): %u\n", c_solucion);
        temperatura_inicial = MU/-log(PHI) * c_solucion;
        temperatura = temperatura_inicial;
        printf("\tTEMPERATURA INICIAL: %.6lf\n", temperatura_inicial);
        do {
            printf("\nITERACION: %d\n", k);
            genera_solucion_candidata();
            delta = c_mejor_vecino - objetivo(sol_act);
            printf("\tDELTA: %d\n", delta);
            printf("\tTEMPERATURA: %.6lf\n", temperatura);
            exponencial = exp(-delta/temperatura);
            printf("\tVALOR DE LA EXPONENCIAL: %.6lf\n", exponencial);
            aleatorio = drand48();
            if(delta < 0 || aleatorio < exponencial) { //se acepta
                for(i = 0; i < n_ciudades - 1; i++) {
                    sol_act[i] = mejor_vecino[i];
                }
                candidatas_acept++;
                candidatas_prob++;
                printf("\tSOLUCION CANDIDATA ACEPTADA\n");

                if(c_mejor_vecino < c_solucion) { //es optimo global
                    for(i = 0; i < n_ciudades - 1; i++) {
                        solucion[i] = sol_act[i];
                    }
                    c_solucion = c_mejor_vecino;
                    ite_solucion = k;
                }
            }
            else { //no se acepta
                candidatas_prob++;
            }
            printf("\tCANDIDATAS PROBADAS: %u, ACEPTADAS: %u\n", candidatas_prob, candidatas_acept);
            if(candidatas_prob == MAX_VECINOS || candidatas_acept == MAX_EXITOS) { //enfriamiento
                enfriar(enfriamientos + 1);
            }

            k++;
        } while(k <= ITE_PARADA);

        printf("\n\nMEJOR SOLUCION: \n");
        printf("\tRECORRIDO:");
        for(i = 0; i < n_ciudades - 1; i++) {
            printf(" %u", solucion[i]);
        }
        printf(" \n");
        printf("\tFUNCION OBJETIVO (km): %u\n", c_solucion);
        printf("\tITERACION: %d\n", ite_solucion);
        printf("\tmu = %lf, phi = %lf\n", MU, PHI);
        printf("ENFRIAMIENTOS: %u\n", enfriamientos);
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

    return c_solucion;
}


//BUSQUEDA SA A PARTIR DE ARCHIVO
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

void genera_solucion_candidata_archivo() {
    double r = 0.0;
    int indice_ciudad, indice_insercion, indice_mejor_ins;
    unsigned int ciudad;
    int i;

    fscanf(archivo_aleatorios, "%lf\r\n", &r);
    indice_ciudad = floor(r * (n_ciudades - 1));
    ciudad = sol_act[indice_ciudad];
    printf("\tINDICE CIUDAD: %d\n", indice_ciudad);
    printf("\tCIUDAD: %u\n", ciudad);

    //inicialmente, el mejor vecino es el primero
    c_vecino = 0;
    indice_insercion = (indice_ciudad + 1) % (n_ciudades - 1);
    insercion(indice_ciudad, indice_insercion);
    for(i = 0; i < n_ciudades - 1; i++) {
        mejor_vecino[i] = s_prima[i];
    }
    c_mejor_vecino = c_vecino;
    indice_mejor_ins = indice_insercion;
    indice_insercion = (indice_insercion + 1) % (n_ciudades - 1);
    //se busca el mejor de todos
    do {
        insercion(indice_ciudad, indice_insercion);
        if(c_vecino < c_mejor_vecino || (c_vecino == c_mejor_vecino && indice_insercion < indice_mejor_ins)) {
            for(i = 0; i < n_ciudades - 1; i++) {
                mejor_vecino[i] = s_prima[i];
            }
            c_mejor_vecino = c_vecino;
            indice_mejor_ins = indice_insercion;
        }

        indice_insercion = (indice_insercion + 1) % (n_ciudades - 1);
    } while(indice_insercion != indice_ciudad);
    printf("\tINDICE INSERCION: %u\n", indice_mejor_ins);
    printf("\tRECORRIDO:");
    for(i = 0; i < n_ciudades - 1; i++) {
        printf(" %u", mejor_vecino[i]);
    }
    printf(" \n");
    printf("\tFUNCION OBJETIVO (km): %u\n", c_mejor_vecino);
}

unsigned int busq_sa_archivo(char* traza) { //devuelve la solucion alcanzada, o 0 en caso de error
    unsigned int c_solucion = 0;

    archivo_aleatorios = fopen(traza, "r");
    if(archivo_aleatorios != NULL) {
        sol_act = (unsigned int *)calloc(n_ciudades - 1, sizeof(unsigned int));
        s_prima = (unsigned int *)calloc(n_ciudades - 1, sizeof(unsigned int));
        mejor_vecino = (unsigned int *)calloc(n_ciudades - 1, sizeof(unsigned int));
        c_vecino = -1;
        c_mejor_vecino = -1;
        enfriamientos = 0;
        candidatas_prob = 0, candidatas_acept = 0;
        if(sol_act != NULL && mejor_vecino != NULL && s_prima != NULL) {
            int i = 0, k = 1, ite_solucion = 0;

            srand48((unsigned)time(NULL));
            genera_solucion_inicial_archivo();
            c_solucion = objetivo(solucion);
            printf("SOLUCION INICIAL:\n");
            printf("\tRECORRIDO:");
            for(i = 0; i < n_ciudades - 1; i++) {
                printf(" %u", solucion[i]);
            }
            printf(" \n");
            for(i = 0; i < n_ciudades - 1; i++) { //primera solucion actual: la solucion inicial
                sol_act[i] = solucion[i];
            }
            printf("\tFUNCION OBJETIVO (km): %u\n", c_solucion);
            temperatura_inicial = MU/-log(PHI) * c_solucion;
            temperatura = temperatura_inicial;
            printf("\tTEMPERATURA INICIAL: %.6lf\n", temperatura_inicial);
            do {
                printf("\nITERACION: %d\n", k);
                genera_solucion_candidata_archivo();
                delta = c_mejor_vecino - objetivo(sol_act);
                printf("\tDELTA: %d\n", delta);
                printf("\tTEMPERATURA: %.6lf\n", temperatura);
                exponencial = exp(-delta/temperatura);
                printf("\tVALOR DE LA EXPONENCIAL: %.6lf\n", exponencial);
                fscanf(archivo_aleatorios, "%lf\r\n",&aleatorio);
                if(delta < 0 || aleatorio < exponencial) { //se acepta
                    for(i = 0; i < n_ciudades - 1; i++) {
                        sol_act[i] = mejor_vecino[i];
                    }
                    candidatas_acept++;
                    candidatas_prob++;
                    printf("\tSOLUCION CANDIDATA ACEPTADA\n");

                    if(c_mejor_vecino < c_solucion) { //es optimo global
                        for(i = 0; i < n_ciudades - 1; i++) {
                            solucion[i] = sol_act[i];
                        }
                        c_solucion = c_mejor_vecino;
                        ite_solucion = k;
                    }
                }
                else { //no se acepta
                    candidatas_prob++;
                }
                printf("\tCANDIDATAS PROBADAS: %u, ACEPTADAS: %u\n", candidatas_prob, candidatas_acept);
                if(candidatas_prob == MAX_VECINOS || candidatas_acept == MAX_EXITOS) { //enfriamiento
                    enfriar(enfriamientos + 1);
                }

                k++;
            } while(k <= ITE_PARADA);

            printf("\n\nMEJOR SOLUCION: \n");
            printf("\tRECORRIDO:");
            for(i = 0; i < n_ciudades - 1; i++) {
                printf(" %u", solucion[i]);
            }
            printf(" \n");
            printf("\tFUNCION OBJETIVO (km): %u\n", c_solucion);
            printf("\tITERACION: %d\n", ite_solucion);
            printf("\tmu = %lf, phi = %lf\n", MU, PHI);
            printf("ENFRIAMIENTOS: %u\n", enfriamientos);
        }

        fclose(archivo_aleatorios);
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
    }

    return c_solucion;
}
