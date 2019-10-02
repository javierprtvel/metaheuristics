#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "problema_viajante.h"

typedef unsigned char** PERMUTACIONES; //matriz binaria que marca las permutaciones generadas (triangular inferior)

PERMUTACIONES p;
SOLUCION s_prima = NULL, mejor_vecino = NULL;
unsigned char quedan_vecinos = (unsigned char) 0;

void inicializa_permutaciones() {
    int i = 0, j = 0;

    p = (unsigned char **)calloc(n_ciudades - 2, sizeof(unsigned char*));
    //printf("Permutaciones:\n");
    for(; i < n_ciudades - 2; i++) {
        p[i] = (unsigned char *)calloc(i + 1, sizeof(unsigned char));
        //printf("[ ");
        for(j = 0; j < i + 1; j++) {
            p[i][j] = (unsigned char) 0; //cero
            //printf("%u ", p[i][j]);
         }
        //printf("]\n");
    }
    //printf("\n");
}

void reinicia_permutaciones() {
    int i = 0, j = 0;

    //printf("Permutaciones reiniciadas\n");
    for(; i < n_ciudades - 2; i++) {
        //printf("[ ");
        for(j = 0; j < i + 1; j++) {
            p[i][j] = (unsigned char) 0; //cero
            //printf("%u ", p[i][j]);
        }
        //printf("]\n");
    }
    //printf("\n");
}

void elimina_permutaciones() {

    if(p != NULL) {
        //printf("Liberando matriz de permutaciones...\n");
        int i = 0;
        for(; i < n_ciudades - 2; i++) {
            if(p[i] != NULL) {
                free(p[i]);
                p[i] = NULL;
            }
        }
        free(p);
        p = NULL;
        //printf("\n");
    }

}

//BUSQUEDA LOCAL ALEATORIA
void genera_solucion_inicial() {
    int i = 0, j = 0;

    for(; i < n_ciudades - 1; i++) {
        sol_act[i] = 1 + floor(drand48() * (n_ciudades - 1));

        for(j = 0; j < i; j++) {
            if(sol_act[i] == sol_act[j]) {
                int repetido = 0;
                do {
                    repetido = 0;
                    sol_act[i] = sol_act[i] % (n_ciudades - 1) + 1;
                    for(j = 0; j < i; j++) {
                        if(sol_act[i] == sol_act[j]) {
                            repetido = 1;
                            break;
                        }
                    }
                } while(repetido);
                break;
            }
        }
    }

    printf("Solución inicial: ");
    for(i = 0; i < n_ciudades - 2; i++) {
        printf("%u-", sol_act[i]);
    }
    printf("%u\n\n", sol_act[i]);
}

void genera_vecino() {
    int i = 0, j = 0;
    unsigned int tmp = -1;

    //permutacion sobre la solucion actual
    i = floor(drand48() * (n_ciudades - 2));
    j = floor(drand48() * (n_ciudades - 2));
    if(i < j) {
        tmp = i;
        i = j;
        j = tmp;
    }
    //printf("Permutación (%u, %u)\n", i, j);

    int i_inicial = i, j_inicial = j;
    while(p[i][j] && quedan_vecinos) { //permutacion ya generada
        j = (j + 1) % (i + 1);
        i = (j == 0)? (i + 1) % (n_ciudades - 2) : i;
        //printf("Permutación (%u, %u)\n", i, j);

        if(i == i_inicial && j == j_inicial) { //se volvio al principio, se ha generado todo el entorno
            quedan_vecinos = (unsigned char) 0;
        }
    }
    if(quedan_vecinos) {
        //asignacion del nuevo vecino a s_prima
        int k = 0;
        for(; k < n_ciudades - 1; k++) {
            s_prima[k] = sol_act[k];
        }
        tmp = s_prima[i + 1];
        s_prima[i + 1] = s_prima[j];
        s_prima[j] = tmp;

        p[i][j] = (unsigned char) 1;
        printf("Permutación (%u, %u)\n", i, j);
    }
}

unsigned int busq_local_aleatoria() { //devuelve la solucion alcanzada, o 0 en caso de error
    unsigned int solucion = 0;

    srand48((unsigned)time(NULL));
    genera_solucion_inicial();
    objetivo(sol_act);

    p = NULL;
    inicializa_permutaciones();
    quedan_vecinos = (unsigned char) 1;
    mejor_vecino = (unsigned int *)calloc(n_ciudades - 1, sizeof(unsigned int));
    s_prima = (unsigned int *)calloc(n_ciudades - 1, sizeof(unsigned int));
    if(p != NULL && mejor_vecino != NULL && s_prima != NULL) {
        int i = 0, j = 0;

        for(; i < n_ciudades - 1; i++) { //inicialmente, el primer mejor es la solucion actual
            mejor_vecino[i] = sol_act[i];
        }
        do {
            for(i = 0; i < n_ciudades - 1; i++) {
                sol_act[i] = mejor_vecino[i]; //solucion actual: el primer mejor de la ultima iteracion
            }

            do {
                genera_vecino();
            } while(objetivo(s_prima) >= objetivo(sol_act) && quedan_vecinos);

            if(objetivo(s_prima) < objetivo(sol_act)) {
                for(i = 0; i < n_ciudades - 1; i++) {
                    mejor_vecino[i] = s_prima[i];
                }
                solucion = objetivo(mejor_vecino);

                printf("Primer mejor vecino encontrado. Distancia: %u\n", solucion);
                printf("Recorrido óptimo: ");
                for(j = 0; j < n_ciudades - 2; j++) {
                    printf("%u-", mejor_vecino[j]);
                }
                printf("%u\n\n", mejor_vecino[j]);

                reinicia_permutaciones();
                quedan_vecinos = (unsigned char) 1;
            }
            else {
                printf("Generado todo el entorno sin éxito\n");
            }
        } while(objetivo(mejor_vecino) < objetivo(sol_act));
        solucion = objetivo(sol_act);
    }

    if(mejor_vecino != NULL) {
        free(mejor_vecino);
        mejor_vecino = NULL;
    }
    if(s_prima != NULL) {
        free(s_prima);
        s_prima = NULL;
    }
    elimina_permutaciones();

    return solucion;
}


//BUSQUEDA LOCAL A PARTIR DE ARCHIVO
FILE *archivo_busqueda = NULL;
unsigned int n_solucion = 0, n_vecino = 0;

void genera_solucion_inicial_archivo() {
    int i = 0, j = 0;
    double r = 0.0;

    for(; i < n_ciudades - 1; i++) {
        fscanf(archivo_busqueda, "%lf\r\n", &r);
        sol_act[i] = 1 + floor(r * (n_ciudades - 1));

        for(j = 0; j < i; j++) {
            if(sol_act[i] == sol_act[j]) {
                int repetido = 0;
                do {
                    repetido = 0;
                    sol_act[i] = sol_act[i] % (n_ciudades - 1) + 1;
                    for(j = 0; j < i; j++) {
                        if(sol_act[i] == sol_act[j]) {
                            repetido = 1;
                            break;
                        }
                    }
                } while(repetido);
                break;
            }
        }
    }
    /*
    printf("Solución inicial: ");
    for(i = 0; i < n_ciudades - 2; i++) {
        printf("%u-", sol_act[i]);
    }
    printf("%u\n\n", sol_act[i]);
    */
}

void genera_vecino_archivo() {
    int i = 0, j = 0;
    unsigned int tmp = -1;
    double r = 0.0;

    fscanf(archivo_busqueda, "%lf\r\n", &r);
    //printf("%lf\n", r);
    i = floor(r * (n_ciudades - 1));
    fscanf(archivo_busqueda, "%lf\r\n", &r);
    //printf("%lf\n", r);
    j = floor(r * (n_ciudades - 1));

    if(i < j) {
        tmp = i;
        i = j;
        j = tmp;
    }
    //repeticion de pares de indices
    while(i == j) {
        i = (i + 1) % (n_ciudades - 1);
        j = (i == 0)? 0 : (j + 1) % i;
        //i = (j == 0)? (i + 1) % (n_ciudades - 1): i;
    }
    i--; //la matriz de permutaciones no tiene diagonal


    //permutacion sobre la solucion actual
    //printf("Permutación (%u, %u)\n", i, j);
    int i_inicial = i, j_inicial = j;
    while(p[i][j] && quedan_vecinos) { //permutacion ya generada
        j = (j + 1) % (i + 1);
        i = (j == 0)? (i + 1) % (n_ciudades - 2) : i;
        //printf("Permutación (%u, %u)\n", i, j);

        if(i == i_inicial && j == j_inicial) { //se volvio al principio, se ha generado todo el entorno
            quedan_vecinos = (unsigned char) 0;
        }
    }
    if(quedan_vecinos) {
        //asignacion del nuevo vecino a s_prima
        int k = 0;
        for(; k < n_ciudades - 1; k++) {
            s_prima[k] = sol_act[k];
        }
        tmp = s_prima[i + 1];
        s_prima[i + 1] = s_prima[j];
        s_prima[j] = tmp;

        printf("\tVECINO V_%u -> Intercambio: (%u, %u); [", n_vecino, i + 1, j);
        for(k = 0; k < n_ciudades - 2; k++) {
            printf("%u, ", s_prima[k]);
        }
        printf("%u]; ", s_prima[n_ciudades - 2]);
        printf("%ukm\n", objetivo(s_prima));

        p[i][j] = (unsigned char) 1;
        n_vecino++;
    }
}

unsigned int busq_local_archivo(char* traza) { //devuelve la solucion alcanzada, o 0 en caso de error
    unsigned int solucion = 0;

    archivo_busqueda = NULL;
    n_solucion = 0;
    n_vecino = 0;

    archivo_busqueda = fopen(traza, "r");
    if(archivo_busqueda != NULL) {
        genera_solucion_inicial_archivo();

        p = NULL;
        inicializa_permutaciones();
        quedan_vecinos = (unsigned char) 1;
        mejor_vecino = (unsigned int *)calloc(n_ciudades - 1, sizeof(unsigned int));
        s_prima = (unsigned int *)calloc(n_ciudades - 1, sizeof(unsigned int));
        if(p != NULL && mejor_vecino != NULL && s_prima != NULL) {
            int i = 0, j = 0;

            for(; i < n_ciudades - 1; i++) { //inicialmente, el primer mejor es la solucion actual
                mejor_vecino[i] = sol_act[i];
            }
            do {
                printf("\nSOLUCION S_%u -> [", n_solucion);
                for(i = 0; i < n_ciudades - 2; i++) {
                    sol_act[i] = mejor_vecino[i]; //solucion actual: el primer mejor de la ultima iteracion
                    printf("%u, ", sol_act[i]);
                }
                sol_act[n_ciudades - 2] = mejor_vecino[n_ciudades - 2];
                printf("%u]; ", sol_act[n_ciudades - 2]);
                printf("%ukm\n", objetivo(sol_act));
                n_solucion++;

                do {
                    genera_vecino_archivo();
                } while(objetivo(s_prima) >= objetivo(sol_act) && quedan_vecinos);

                if(objetivo(s_prima) < objetivo(sol_act)) {
                    for(i = 0; i < n_ciudades - 1; i++) {
                        mejor_vecino[i] = s_prima[i];
                    }
                    solucion = objetivo(mejor_vecino);
                    /*
                    printf("Primer mejor vecino encontrado. Distancia: %u\n", solucion);
                    printf("Recorrido óptimo: ");
                    for(j = 0; j < n_ciudades - 2; j++) {
                        printf("%u-", mejor_vecino[j]);
                    }
                    printf("%u\n\n", mejor_vecino[j]);
                    */
                    reinicia_permutaciones();
                    quedan_vecinos = (unsigned char) 1;
                    n_vecino = 0;
                }
                else {
                    //printf("Generado todo el entorno sin éxito\n");
                }
            } while(objetivo(mejor_vecino) < objetivo(sol_act));
            solucion = objetivo(sol_act);
        }

        if(mejor_vecino != NULL) {
            free(mejor_vecino);
            mejor_vecino = NULL;
        }
        if(s_prima != NULL) {
            free(s_prima);
            s_prima = NULL;
        }
        elimina_permutaciones();

        fclose(archivo_busqueda);
        archivo_busqueda = NULL;
    }

    return solucion;
}
