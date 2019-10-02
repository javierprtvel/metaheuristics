#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "problema_viajante.h"

#define SORT_TYPE SOLUCION
#define SORT_NAME poblacion
#define SORT_CMP(x, y) ((objetivo(x)) < (objetivo(y)) ? -1 : ((objetivo(x)) == (objetivo(y)) ? 0 : 1))
#include "sort.h"

#define N_POBLACION 100
#define N_ELITISMO 2
#define POBLACION_PARADA 1000
#define PROB_CRUCE 0.9
#define PROB_MUTA 0.01

typedef struct {
    SOLUCION genotipos[N_POBLACION];
} POBLACION;

POBLACION poblacion_inicial, poblacion_prima;
SOLUCION mejores[N_ELITISMO];
unsigned int torneo = 0;

//BUSQUEDA CE
void genera_solucion_aleatoria(SOLUCION genotipo) {
    int i = 0, j = 0;

    for(; i < n_ciudades - 1; i++) {
        genotipo[i] = 1 + (int)(floor(drand48() * (n_ciudades - 1))) % (n_ciudades - 1);

        for(j = 0; j < i; j++) {
            if(genotipo[i] == genotipo[j]) {
                int repetido = 0;
                do {
                    repetido = 0;
                    genotipo[i] = genotipo[i] % (n_ciudades - 1) + 1;
                    for(j = 0; j < i; j++) {
                        if(genotipo[i] == genotipo[j]) {
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

void genera_solucion_voraz(SOLUCION genotipo) {
    int i = 0, j = 0, k = 0;
    int repetido = 0;
    unsigned int ciudad_origen = 0, ciudad_dist_min = 0, dist_min = 0;

    //primer gen con alelo aleatorio, los demas con alelo segun minima distancia
    genotipo[0] = 1 + (int)(floor(drand48() * (n_ciudades - 1))) % (n_ciudades - 1);
    for(i = 1; i < n_ciudades - 1; i++) {
        ciudad_origen = genotipo[i - 1];
        //printf("Ciudad origen: %u\n", ciudad_origen);

        //no se tienen en cuenta alelos ya asignados (ciudades ya incluidas en el recorrido)
        ciudad_dist_min = 0;
        do {
            ciudad_dist_min = ciudad_dist_min % (n_ciudades - 1) + 1;
            repetido = 0;
            for(k = 0; k < i; k++) {
                if(ciudad_dist_min == genotipo[k]) {
                    repetido = 1;
                    break;
                }
            }
        } while(repetido);
        //printf("Ciudad dist_min: %u\n", ciudad_dist_min);
        dist_min = (ciudad_origen > ciudad_dist_min)? d[ciudad_origen - 1][ciudad_dist_min]
            : d[ciudad_dist_min - 1][ciudad_origen];
        genotipo[i] = ciudad_dist_min;

        j = 1;
        while(j < (n_ciudades - i - 1)) {
            //no se tienen en cuenta alelos ya asignados (ciudades ya incluidas en el genotipo)
            do {
                repetido = 0;
                ciudad_dist_min = ciudad_dist_min % (n_ciudades - 1) + 1;
                for(k = 0; k < i; k++) {
                    if(ciudad_dist_min == genotipo[k]) {
                        repetido = 1;
                        break;
                    }
                }
            } while(repetido);
            //printf("Ciudad dist_min: %u\n", ciudad_dist_min);

            if(ciudad_origen > ciudad_dist_min && d[ciudad_origen - 1][ciudad_dist_min] < dist_min) {
                genotipo[i] = ciudad_dist_min;
                dist_min = d[ciudad_origen - 1][ciudad_dist_min];
            }
            else if(ciudad_origen < ciudad_dist_min && d[ciudad_dist_min - 1][ciudad_origen] < dist_min) {
                genotipo[i] = ciudad_dist_min;
                dist_min = d[ciudad_dist_min - 1][ciudad_origen];
            }

            j++;
        }
        //printf("Ciudad escogida: %u\n", genotipo[i]);
    }
}

int genera_poblacion_inicial() {
    int exito = 0;

    unsigned int c_solucion = -1;
    int i = 0;
    //genotipos aleatorios
    poblacion_inicial.genotipos[0] = (SOLUCION)calloc((n_ciudades - 1), sizeof(unsigned int));
    if(poblacion_inicial.genotipos[0] == NULL) {
        return exito;
    }
    genera_solucion_aleatoria(poblacion_inicial.genotipos[0]);
    c_solucion = objetivo(poblacion_inicial.genotipos[0]);
    for(i = 1; i < N_POBLACION/2; i++) {
        poblacion_inicial.genotipos[i] = (SOLUCION)calloc((n_ciudades - 1), sizeof(unsigned int));
        if(poblacion_inicial.genotipos[i] == NULL) {
            break;
        }
        genera_solucion_aleatoria(poblacion_inicial.genotipos[i]);

        if(objetivo(poblacion_inicial.genotipos[i]) < c_solucion) { //mejor solucion de la poblacion
            memcpy(solucion, poblacion_inicial.genotipos[i], (n_ciudades - 1) * sizeof(unsigned int));
            c_solucion = objetivo(solucion);
        }
    }
    //genotipos semialeatorios
    for(; i < N_POBLACION; i++) {
        poblacion_inicial.genotipos[i] = (SOLUCION)calloc((n_ciudades - 1), sizeof(unsigned int));
        if(poblacion_inicial.genotipos[i] == NULL) {
            break;
        }
        genera_solucion_voraz(poblacion_inicial.genotipos[i]);

        if(objetivo(poblacion_inicial.genotipos[i]) < c_solucion) { //mejor solucion de la poblacion
            memcpy(solucion, poblacion_inicial.genotipos[i], (n_ciudades - 1) * sizeof(unsigned int));
            c_solucion = objetivo(solucion);
        }
    }

    if(i == N_POBLACION) {
        exito = 1;
    }

    return exito;
}

void destruye_poblacion(POBLACION *pob) {
    int i = 0;

    for(; i < N_POBLACION; i++) {
        if(pob->genotipos[i] != NULL) {
            free(pob->genotipos[i]);
            pob->genotipos[i] = NULL;
        }
    }
}

void imprime_poblacion(POBLACION pob) {
    int i = 0, j = 0;

    for(; i < N_POBLACION; i++) {
        printf("INDIVIDUO %d = {FUNCION OBJETIVO (km): %u, RECORRIDO: ", i, objetivo(pob.genotipos[i]));
        for(j = 0; j < n_ciudades - 1; j++) {
            printf("%u ", pob.genotipos[i][j]);
        }
        printf("}\n");
    }
}

int selecciona_padre(unsigned int torneo) {
    int i = 0, j = 0;
    int ind_padre = 0;

    /* torneo de k=2 genotipos aleatorios (deberia programarse con vectores y bucles parametrizados con k
       para hacerlo generico)
    */
    i = floor(drand48() * (N_POBLACION - 1));
    j = floor(drand48() * (N_POBLACION - 1));
    if(objetivo(poblacion_inicial.genotipos[i]) <= objetivo(poblacion_inicial.genotipos[j])) {
        ind_padre = i;
    }
    else {
        ind_padre = j;
    }
    printf("\tTORNEO %u: %d %d GANA %d\n", torneo, i, j, ind_padre);

    return ind_padre;
}

void cruce(int ind_1, int ind_2, int ite) {
    SOLUCION p1 = poblacion_inicial.genotipos[ind_1];
    SOLUCION p2 = poblacion_inicial.genotipos[ind_2];
    SOLUCION h1 = poblacion_prima.genotipos[ite + N_ELITISMO];
    SOLUCION h2 = poblacion_prima.genotipos[ite + N_ELITISMO + 1];
    double aleatorio = drand48();
    int pto_1 = 0, pto_2 = 0, tmp = 0;
    int n_area_corte = 0;
    int i = 0, j = 0, k1 = 0, k2 = 0, z = 0;
    int rep = 0;

    printf("\tCRUCE: (%d, %d) (ALEATORIO: %lf)\n", ite, ite + 1, aleatorio);
    printf("\t\tPADRE: = {FUNCION OBJETIVO (km): %u, RECORRIDO: ", objetivo(p1));
    for(i = 0; i < n_ciudades - 1; i++) {
        printf("%u ", p1[i]);
    }
    printf("}\n");
    printf("\t\tPADRE: = {FUNCION OBJETIVO (km): %u, RECORRIDO: ", objetivo(p2));
    for(i = 0; i < n_ciudades - 1; i++) {
        printf("%u ", p2[i]);
    }
    printf("}\n");

    if(aleatorio <= PROB_CRUCE) {
        pto_1 = (int)floor(drand48() * (n_ciudades - 1)) % (n_ciudades - 1);
        pto_2 = (int)floor(drand48() * (n_ciudades - 1)) % (n_ciudades - 1);
        printf("\t\tCORTES: (%d, %d)\n", pto_1, pto_2);
        if(pto_1 > pto_2) {
            tmp = pto_1;
            pto_1 = pto_2;
            pto_2 = tmp;
        }
        n_area_corte = pto_2 - pto_1 + 1;
        //printf("\t\tAREA CRUCE: %d\n", n_area_corte);
        //printf("\t\tPOSICIONES RESTANTES: %d\n", (n_ciudades - 1) - (pto_2 - pto_1 + 1));

        memcpy(h1 + pto_1, p1 + pto_1, n_area_corte * sizeof(unsigned int));
        memcpy(h2 + pto_1, p2 + pto_1, n_area_corte * sizeof(unsigned int));
        i = 0;
        k1 = (pto_2 + 1) % (n_ciudades - 1), k2 = k1, z = k1;
        while(i < (n_ciudades - 1) - n_area_corte) {
            //herencia p2-h1
            do {
                rep = 0;
                for(j = pto_1; j <= pto_2; j++) {
                    if(p2[k1] == h1[j]) {
                        k1= (k1 + 1) % (n_ciudades - 1);
                        rep = 1;
                        break;
                    }
                }
            } while(rep);
            h1[z] = p2[k1];
            //herencia p1-h//se generan los genotipos de la siguiente poblacion por torneo k=2
            do {
                rep = 0;
                for(j = pto_1; j <= pto_2; j++) {
                    if(p1[k2] == h2[j]) {
                        k2 = (k2 + 1) % (n_ciudades - 1);
                        rep = 1;
                        break;
                    }
                }
            } while(rep);
            h2[z] = p1[k2];

            z = (z + 1) % (n_ciudades - 1);
            k1 = (k1 + 1) % (n_ciudades - 1);
            k2 = (k2 + 1) % (n_ciudades - 1);
            i++;
        }

        printf("\t\tHIJO: = {FUNCION OBJETIVO (km): %u, RECORRIDO: ", objetivo(h1));
        for(i = 0; i < n_ciudades - 1; i++) {
            printf("%u ", h1[i]);
        }
        printf("}\n");
        printf("\t\tHIJO: = {FUNCION OBJETIVO (km): %u, RECORRIDO: ", objetivo(h2));
        for(i = 0; i < n_ciudades - 1; i++) {
            printf("%u ", h2[i]);
        }
        printf("}\n");
    }
    else { //hijos identicos a los padres
        printf("\t\tNO SE CRUZA\n");
        memcpy(h1, p1, (n_ciudades - 1) * sizeof(unsigned int));
        memcpy(h2, p2, (n_ciudades - 1) * sizeof(unsigned int));
    }
    printf("\n");
}

void intercambio(SOLUCION *s, int i, int j) {
    unsigned int tmp;

    tmp = (*s)[i];
    (*s)[i] = (*s)[j];
    (*s)[j] = tmp;
}

void mutacion(int ind_hijo) {
    SOLUCION hijo = poblacion_prima.genotipos[ind_hijo];
    double aleatorio = 0.0;
    int pos_i = 0, pos_j = 0;
    int i = 0;

    printf("\tINDIVIDUO %d\n", ind_hijo - N_ELITISMO);
    printf("\tRECORRIDO ANTES: ");
    for(; i < n_ciudades - 1; i++) {
        printf("%u ", hijo[i]);
    }
    printf("\n");

    for(i = 0; i < n_ciudades - 1; i++) {
        aleatorio = drand48();
        if(aleatorio <= PROB_MUTA) {
            pos_i = i;
            pos_j = (int)floor(drand48() * (n_ciudades - 1)) % (n_ciudades - 1);
            printf("\t\tPOSICION: %d (ALEATORIO %lf) INTERCAMBIO CON: %d\n", i, aleatorio, pos_j);
            intercambio(&hijo, pos_i, pos_j);
        }
        else {
            //no muta
            printf("\t\tPOSICION: %d (ALEATORIO %lf) NO MUTA\n", i, aleatorio);
        }
    }

    printf("\tRECORRIDO DESPUES: ");
    for(i = 0; i < n_ciudades - 1; i++) {
        printf("%u ", hijo[i]);
    }
    printf("\n");
    printf("\n");
}

void reemplazo() {
    /*Escoger los k mejores genotipos de la poblacion inicial e insertarlos en orden
      decreciente al principio de poblacion inicial.
    */
    SOLUCION mejor = NULL, aux = NULL;
    unsigned int fitness_mejor = 0, fitness_aux;
    int i = 0, j = 0, k = 0, l = 0;
    int rep = 0;
    memset(mejores, 0, N_ELITISMO * sizeof(SOLUCION));
    for(i = 0; i < N_ELITISMO; i++) {
        //ignorar los primeros individuos que ya estan en el vector mejores
        j = -1;
        do {
            j++;
            rep = 0;
            for(k = 0; k < N_ELITISMO && !rep; k++) {
                if(poblacion_inicial.genotipos[j] == mejores[k]) { //mismo individuo
                    rep = 1;
                }
                /*
                else if(mejores[k] != NULL) {
                    l = 0;
                    while(l < n_ciudades - 1 && poblacion_inicial.genotipos[j][l] == mejores[k][l]) {
                        //printf("INDIVIDUO INICIAL: %d, POSICION %d ; INDIVIDUO ELITE: %d, POSICION %d\n", j, l, k, l);
                        l++;
                    }
                    rep = (l == n_ciudades - 1)? 1 : 0; //diferente individuo, mismo genotipo
                }
                */
            }
        } while(rep);
        mejor = poblacion_inicial.genotipos[j];
        fitness_mejor = objetivo(mejor);

        //valores iniciales establecidos: calcular siguiente minimo
        for(; j < N_POBLACION; j++) {
            aux = poblacion_inicial.genotipos[j];
            fitness_aux = objetivo(aux);
            rep = 0;
            for(k = 0; k < N_ELITISMO && !rep; k++) {
                if(aux == mejores[k]) {
                    rep = 1;
                }
                /*
                else if(mejores[k] != NULL) {
                    l = 0;
                    while(l < n_ciudades - 1 && aux[l] == mejores[k][l]) {
                        //printf("INDIVIDUO INICIAL: %d, POSICION %d ; INDIVIDUO ELITE: %d, POSICION %d\n", j, l, k, l);
                        l++;
                    }
                    rep = (l == n_ciudades - 1)? 1 : 0; //diferente individuo, mismo genotipo
                }
                */
            }
            if(!rep && fitness_aux < fitness_mejor) {
                mejor = aux;
                fitness_mejor = fitness_aux;
            }

            //se anhade el genotipo al vector de mejores
            mejores[N_ELITISMO - 1 - i] = mejor;
        }
    }
    /*
    printf("MEJORES (ELITISMO):\n");
    for(i = 0; i < N_ELITISMO; i++) {
        printf("INDIVIDUO %d: {FUNCION OBJETIVO (km): %u, RECORRIDO: ", i, objetivo(mejores[i]));
        for(j = 0; j < n_ciudades - 1; j++) {
            printf("%u ", mejores[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    */

    //asignar los mejores en las primeras posiciones de la poblacion inicial de la siguiente iteracion
    for(i = 0; i < N_ELITISMO; i++) {
        memmove(poblacion_inicial.genotipos[i], mejores[i], (n_ciudades - 1) * sizeof(unsigned int));
    }

    //ordenar por fitness creciente los hijos (poblacion_prima)
    SOLUCION *ptr = &(poblacion_prima.genotipos[N_ELITISMO]);
    poblacion_tim_sort(ptr, N_POBLACION - N_ELITISMO); //libreria externa
    for(i = N_ELITISMO; i < N_POBLACION; i++) {
        memcpy(poblacion_inicial.genotipos[i], poblacion_prima.genotipos[i], (n_ciudades - 1) * sizeof(unsigned int));
  }
}

unsigned int busq_ce() { //devuelve la solucion alcanzada, o 0 en caso de error
    unsigned int c_solucion = 0;
    int i = 0;

    printf("POBLACION INICIAL\n");
    srand48((unsigned)time(NULL));
    int exito = genera_poblacion_inicial();
    if(!exito) {
        printf("ERROR GENERANDO POBLACIÓN INICIAL!\n");
    }
    else {
        //inicializar poblacion_prima que almacena temporalmente los genotipos descendientes
        for(i = N_ELITISMO; i < N_POBLACION; i++) {
            poblacion_prima.genotipos[i] = (SOLUCION)calloc(n_ciudades - 1, sizeof(unsigned int));
            if(poblacion_prima.genotipos[i] == NULL) {
                break;
            }
        }
        if(i < N_POBLACION) {
            printf("ERROR INICIALIZANDO POBLACIÓN PRIMA!\n");
        }
        else {
            imprime_poblacion(poblacion_inicial);
            printf("\n");

            c_solucion = objetivo(solucion);
            int padres[N_POBLACION - N_ELITISMO];
            SOLUCION mejor_elite = NULL, mejor_desc= NULL, mejor_pob = NULL;
            unsigned int pob_generadas = 1, pob_mejor_sol = pob_generadas - 1;
            while(pob_generadas <= POBLACION_PARADA) {
                /*PARA ADECUAR LA SALIDA POR PANTALLA DE LA EJECUCION DEL ALGORITMO CON EL FORMATO INDICADO
                  EN LA TRAZA MODELO, LA SELECCION, CRUCE, MUTACION Y REEMPLAZO SE IMPLEMENTAN EN BUCLES
                  SEPARADOS.
                  SI NO SE REQUIRIESE ESE FORMATO, SE PODRIA EJECUTAR UN UNICO BUCLE DE (N_POBLACION-N_ELITISMO)/2
                  ITERACIONES EN EL QUE EN CADA ITERACION SE HICIESE SELECCION Y CRUCE DE DOS PADRES Y
                  MUTACION DE LOS DOS HIJOS. POSTERIORMENTE SE HARIA EL REEMPLAZO.
                */
                printf("ITERACION: %u, SELECCION\n", pob_generadas);
                for(i = 0; i < N_POBLACION - N_ELITISMO; i++) {
                    //se generan los genotipos de la siguiente poblacion por torneo k=2
                    padres[i] = selecciona_padre(i);
                }
                printf("\n");

                printf("ITERACION: %u, CRUCE\n", pob_generadas);
                for(i = 0; i < N_POBLACION - N_ELITISMO; i += 2) {
                    cruce(padres[i], padres[i + 1], i);
                }

                printf("ITERACION: %u, MUTACION\n", pob_generadas);
                for(i = N_ELITISMO; i < N_POBLACION; i++) {
                    mutacion(i);
                }
                printf("\n");

                printf("ITERACION: %u, REEMPLAZO\n", pob_generadas);
                reemplazo();
                imprime_poblacion(poblacion_inicial);
                printf("\n");

                //optimo global
                mejor_elite = poblacion_inicial.genotipos[N_ELITISMO - 1];
                mejor_desc = poblacion_inicial.genotipos[N_ELITISMO];
                mejor_pob = (objetivo(mejor_elite) <= objetivo(mejor_desc))?
                    mejor_elite : mejor_desc;
                if(objetivo(mejor_pob) < c_solucion) {
                    memcpy(solucion, mejor_pob, (n_ciudades - 1) * sizeof(unsigned int));
                    c_solucion = objetivo(mejor_pob);
                    pob_mejor_sol = pob_generadas;
                }

                pob_generadas++;
            }

            printf("\nMEJOR SOLUCION: \n");
            printf("RECORRIDO: ");
            for(i = 0; i < n_ciudades - 1; i++) {
                printf("%u ", solucion[i]);
            }
            printf("\nFUNCION OBJETIVO (km): %u\n", c_solucion);
            printf("ITERACION: %u\n", pob_mejor_sol);
        }
    }
    destruye_poblacion(&poblacion_prima);
    destruye_poblacion(&poblacion_inicial);

    return c_solucion;
}


//BUSQUEDA CE A PARTIR DE ARCHIVO
FILE *archivo_aleatorios = NULL;

void genera_solucion_aleatoria_arch(SOLUCION genotipo) {
    int i = 0, j = 0;
    double r = 0.0;
    for(; i < n_ciudades - 1; i++) {
        fscanf(archivo_aleatorios, "%lf\r\n", &r);
        genotipo[i] = 1 + floor(r * (n_ciudades - 1));

        for(j = 0; j < i; j++) {
            if(genotipo[i] == genotipo[j]) {
                int repetido = 0;
                do {
                    repetido = 0;
                    genotipo[i] = genotipo[i] % (n_ciudades - 1) + 1;
                    for(j = 0; j < i; j++) {
                        if(genotipo[i] == genotipo[j]) {
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

void genera_solucion_voraz_arch(SOLUCION genotipo) {
    int i = 0, j = 0, k = 0;
    double r = 0.0;
    int repetido = 0;
    unsigned int ciudad_origen = 0, ciudad_dist_min = 0, dist_min = 0;

    //primer gen con alelo aleatorio, los demas con alelo segun minima distancia
    fscanf(archivo_aleatorios, "%lf\r\n", &r);
    genotipo[0] = 1 + floor(r * (n_ciudades - 1));
    for(i = 1; i < n_ciudades - 1; i++) {
        ciudad_origen = genotipo[i - 1];
        //printf("Ciudad origen: %u\n", ciudad_origen);

        //no se tienen en cuenta alelos ya asignados (ciudades ya incluidas en el recorrido)
        ciudad_dist_min = ciudad_origen;
        do {
            ciudad_dist_min = ciudad_dist_min % (n_ciudades - 1) + 1;
            repetido = 0;
            for(k = 0; k < i; k++) {
                if(ciudad_dist_min == genotipo[k]) {
                    repetido = 1;
                    break;
                }
            }
        } while(repetido);
        //printf("Ciudad dist_min: %u\n", ciudad_dist_min);
        dist_min = (ciudad_origen > ciudad_dist_min)? d[ciudad_origen - 1][ciudad_dist_min]
            : d[ciudad_dist_min - 1][ciudad_origen];
        genotipo[i] = ciudad_dist_min;

        j = 1;
        while(j < (n_ciudades - i - 1)) {
            //no se tienen en cuenta alelos ya asignados (ciudades ya incluidas en el genotipo)
            do {
                repetido = 0;
                ciudad_dist_min = ciudad_dist_min % (n_ciudades - 1) + 1;
                for(k = 0; k < i; k++) {
                    if(ciudad_dist_min == genotipo[k]) {
                        repetido = 1;
                        break;
                    }
                }
            } while(repetido);
            //printf("Ciudad dist_min: %u\n", ciudad_dist_min);

            if(ciudad_origen > ciudad_dist_min && d[ciudad_origen - 1][ciudad_dist_min] < dist_min) {
                genotipo[i] = ciudad_dist_min;
                dist_min = d[ciudad_origen - 1][ciudad_dist_min];
            }
            else if(ciudad_origen < ciudad_dist_min && d[ciudad_dist_min - 1][ciudad_origen] < dist_min) {
                genotipo[i] = ciudad_dist_min;
                dist_min = d[ciudad_dist_min - 1][ciudad_origen];
            }

            j++;
        }
        //printf("Ciudad escogida: %u\n", genotipo[i]);
    }
}

int genera_poblacion_inicial_arch() {
    int exito = 0;

    unsigned int c_solucion = -1;
    int i = 0;
    //genotipos aleatorios
    poblacion_inicial.genotipos[0] = (SOLUCION)calloc((n_ciudades - 1), sizeof(unsigned int));
    if(poblacion_inicial.genotipos[0] == NULL) {
        return exito;
    }
    genera_solucion_aleatoria_arch(poblacion_inicial.genotipos[0]);
    c_solucion = objetivo(poblacion_inicial.genotipos[0]);
    for(i = 1; i < N_POBLACION/2; i++) {
        poblacion_inicial.genotipos[i] = (SOLUCION)calloc((n_ciudades - 1), sizeof(unsigned int));
        if(poblacion_inicial.genotipos[i] == NULL) {
            break;
        }
        genera_solucion_aleatoria_arch(poblacion_inicial.genotipos[i]);

        if(objetivo(poblacion_inicial.genotipos[i]) < c_solucion) { //mejor solucion de la poblacion
            memcpy(solucion, poblacion_inicial.genotipos[i], (n_ciudades - 1) * sizeof(unsigned int));
            c_solucion = objetivo(solucion);
        }
    }
    //genotipos semialeatorios
    for(; i < N_POBLACION; i++) {
        poblacion_inicial.genotipos[i] = (SOLUCION)calloc((n_ciudades - 1), sizeof(unsigned int));
        if(poblacion_inicial.genotipos[i] == NULL) {
            break;
        }
        genera_solucion_voraz_arch(poblacion_inicial.genotipos[i]);

        if(objetivo(poblacion_inicial.genotipos[i]) < c_solucion) { //mejor solucion de la poblacion
            memcpy(solucion, poblacion_inicial.genotipos[i], (n_ciudades - 1) * sizeof(unsigned int));
            c_solucion = objetivo(solucion);
        }
    }

    if(i == N_POBLACION) {
        exito = 1;
    }

    return exito;
}

int selecciona_padre_arch(unsigned int torneo) {
    int i = 0, j = 0;
    double r = 0.0;
    int ind_padre = 0;

    /* torneo de k=2 genotipos aleatorios (deberia programarse con vectores y bucles parametrizados con k
       para hacerlo generico)
    */
    fscanf(archivo_aleatorios, "%lf\r\n", &r);
    i = floor(r * N_POBLACION);
    fscanf(archivo_aleatorios, "%lf\r\n", &r);
    j = floor(r * N_POBLACION);
    if(objetivo(poblacion_inicial.genotipos[i]) <= objetivo(poblacion_inicial.genotipos[j])) {
        ind_padre = i;
    }
    else {
        ind_padre = j;
    }
    printf("\tTORNEO %u: %d %d GANA %d\n", torneo, i, j, ind_padre);

    return ind_padre;
}

void cruce_arch(int ind_1, int ind_2, int ite) {
    SOLUCION p1 = poblacion_inicial.genotipos[ind_1];
    SOLUCION p2 = poblacion_inicial.genotipos[ind_2];
    SOLUCION h1 = poblacion_prima.genotipos[ite + N_ELITISMO];
    SOLUCION h2 = poblacion_prima.genotipos[ite + N_ELITISMO + 1];
    double r = 0.0;
    int pto_1 = 0, pto_2 = 0, tmp = 0;
    int n_area_corte = 0;
    int i = 0, j = 0, k1 = 0, k2 = 0, z = 0;
    int rep = 0;

    fscanf(archivo_aleatorios, "%lf\r\n", &r);
    printf("\tCRUCE: (%d, %d) (ALEATORIO: %lf)\n", ite, ite + 1, r);
    printf("\t\tPADRE: = {FUNCION OBJETIVO (km): %u, RECORRIDO: ", objetivo(p1));
    for(i = 0; i < n_ciudades - 1; i++) {
        printf("%u ", p1[i]);
    }
    printf("}\n");
    printf("\t\tPADRE: = {FUNCION OBJETIVO (km): %u, RECORRIDO: ", objetivo(p2));
    for(i = 0; i < n_ciudades - 1; i++) {
        printf("%u ", p2[i]);
    }
    printf("}\n");

    if(r <= PROB_CRUCE) {
        fscanf(archivo_aleatorios, "%lf\r\n", &r);
        pto_1 = floor(r * (n_ciudades - 1));
        fscanf(archivo_aleatorios, "%lf\r\n", &r);
        pto_2 = floor(r * (n_ciudades - 1));
        printf("\t\tCORTES: (%d, %d)\n", pto_1, pto_2);
        if(pto_1 > pto_2) {
            tmp = pto_1;
            pto_1 = pto_2;
            pto_2 = tmp;
        }
        n_area_corte = pto_2 - pto_1 + 1;
        //printf("\t\tAREA CRUCE: %d\n", n_area_corte);
        //printf("\t\tPOSICIONES RESTANTES: %d\n", (n_ciudades - 1) - (pto_2 - pto_1 + 1));

        memcpy(h1 + pto_1, p1 + pto_1, n_area_corte * sizeof(unsigned int));
        memcpy(h2 + pto_1, p2 + pto_1, n_area_corte * sizeof(unsigned int));
        i = 0;
        k1 = (pto_2 + 1) % (n_ciudades - 1), k2 = k1, z = k1;
        while(i < (n_ciudades - 1) - n_area_corte) {
            //herencia p2-h1
            do {
                rep = 0;
                for(j = pto_1; j <= pto_2; j++) {
                    if(p2[k1] == h1[j]) {
                        k1= (k1 + 1) % (n_ciudades - 1);
                        rep = 1;
                        break;
                    }
                }
            } while(rep);
            h1[z] = p2[k1];
            //herencia p1-h//se generan los genotipos de la siguiente poblacion por torneo k=2
            do {
                rep = 0;
                for(j = pto_1; j <= pto_2; j++) {
                    if(p1[k2] == h2[j]) {
                        k2 = (k2 + 1) % (n_ciudades - 1);
                        rep = 1;
                        break;
                    }
                }
            } while(rep);
            h2[z] = p1[k2];

            z = (z + 1) % (n_ciudades - 1);
            k1 = (k1 + 1) % (n_ciudades - 1);
            k2 = (k2 + 1) % (n_ciudades - 1);
            i++;
        }

        printf("\t\tHIJO: = {FUNCION OBJETIVO (km): %u, RECORRIDO: ", objetivo(h1));
        for(i = 0; i < n_ciudades - 1; i++) {
            printf("%u ", h1[i]);
        }
        printf("}\n");
        printf("\t\tHIJO: = {FUNCION OBJETIVO (km): %u, RECORRIDO: ", objetivo(h2));
        for(i = 0; i < n_ciudades - 1; i++) {
            printf("%u ", h2[i]);
        }
        printf("}\n");
    }
    else { //hijos identicos a los padres
        printf("\t\tNO SE CRUZA\n");
        memcpy(h1, p1, (n_ciudades - 1) * sizeof(unsigned int));
        memcpy(h2, p2, (n_ciudades - 1) * sizeof(unsigned int));
    }
    printf("\n");
}

void mutacion_arch(int ind_hijo) {
    SOLUCION hijo = poblacion_prima.genotipos[ind_hijo];
    double r = 0.0, aleatorio = 0.0;
    int pos_i = 0, pos_j = 0;
    int i = 0;

    printf("\tINDIVIDUO %d\n", ind_hijo - N_ELITISMO);
    printf("\tRECORRIDO ANTES: ");
    for(; i < n_ciudades - 1; i++) {
        printf("%u ", hijo[i]);
    }
    printf("\n");

    for(i = 0; i < n_ciudades - 1; i++) {
        fscanf(archivo_aleatorios, "%lf\r\n", &aleatorio);
        if(aleatorio <= PROB_MUTA) {
            pos_i = i;
            fscanf(archivo_aleatorios, "%lf\r\n", &r);
            pos_j = floor(r * (n_ciudades - 1));
            printf("\t\tPOSICION: %d (ALEATORIO %lf) INTERCAMBIO CON: %d\n", i, aleatorio, pos_j);
            intercambio(&hijo, pos_i, pos_j);
        }
        else {
            //no muta
            printf("\t\tPOSICION: %d (ALEATORIO %lf) NO MUTA\n", i, aleatorio);
        }
    }

    printf("\tRECORRIDO DESPUES: ");
    for(i = 0; i < n_ciudades - 1; i++) {
        printf("%u ", hijo[i]);
    }
    printf("\n");
    printf("\n");
}

unsigned int busq_ce_archivo(char* traza) { //devuelve la solucion alcanzada, o 0 en caso de error
    unsigned int c_solucion = 0;

    archivo_aleatorios = fopen(traza, "r");
    if(archivo_aleatorios != NULL) {
        printf("POBLACION INICIAL\n");
        srand48((unsigned)time(NULL));
        int exito = genera_poblacion_inicial_arch();
        if(!exito) {
            printf("ERROR GENERANDO POBLACIÓN INICIAL!\n");
        }
        else {
            int i = 0;

            //inicializar poblacion_prima que almacena temporalmente los genotipos descendientes
            for(i = N_ELITISMO; i < N_POBLACION; i++) {
                poblacion_prima.genotipos[i] = (SOLUCION)calloc(n_ciudades - 1, sizeof(unsigned int));
                if(poblacion_prima.genotipos[i] == NULL) {
                    break;
                }
            }
            if(i < N_POBLACION) {
                printf("ERROR INICIALIZANDO POBLACIÓN PRIMA!\n");
            }
            else {
                imprime_poblacion(poblacion_inicial);
                printf("\n");

                c_solucion = objetivo(solucion);
                int padres[N_POBLACION - N_ELITISMO];
                SOLUCION mejor_elite = NULL, mejor_desc= NULL, mejor_pob = NULL;
                unsigned int pob_generadas = 1, pob_mejor_sol = pob_generadas - 1;
                while(pob_generadas <= POBLACION_PARADA) {
                    /*PARA ADECUAR LA SALIDA POR PANTALLA DE LA EJECUCION DEL ALGORITMO CON EL FORMATO INDICADO
                      EN LA TRAZA MODELO, LA SELECCION, CRUCE, MUTACION Y REEMPLAZO SE IMPLEMENTAN EN BUCLES
                      SEPARADOS.
                      SI NO SE REQUIRIESE ESE FORMATO, SE PODRIA EJECUTAR UN UNICO BUCLE DE (N_POBLACION-N_ELITISMO)/2
                      ITERACIONES EN EL QUE EN CADA ITERACION SE HICIESE SELECCION Y CRUCE DE DOS PADRES Y
                      MUTACION DE LOS DOS HIJOS. POSTERIORMENTE SE HARIA EL REEMPLAZO.
                    */
                    printf("ITERACION: %u, SELECCION\n", pob_generadas);
                    for(i = 0; i < N_POBLACION - N_ELITISMO; i++) {
                        //se generan los genotipos de la siguiente poblacion por torneo k=2
                        padres[i] = selecciona_padre_arch(i);
                    }
                    printf("\n");

                    printf("ITERACION: %u, CRUCE \n", pob_generadas);
                    for(i = 0; i < N_POBLACION - N_ELITISMO; i += 2) {
                        cruce_arch(padres[i], padres[i + 1], i);
                    }

                    printf("ITERACION: %u, MUTACION\n", pob_generadas);
                    for(i = N_ELITISMO; i < N_POBLACION; i++) {
                        mutacion_arch(i);
                    }
                    printf("\n");

                    printf("ITERACION: %u, REEMPLAZO\n", pob_generadas);
                    reemplazo();
                    imprime_poblacion(poblacion_inicial);
                    printf("\n");

                    //optimo global
                    mejor_elite = poblacion_inicial.genotipos[N_ELITISMO - 1];
                    mejor_desc = poblacion_inicial.genotipos[N_ELITISMO];
                    mejor_pob = (objetivo(mejor_elite) <= objetivo(mejor_desc))?
                        mejor_elite : mejor_desc;
                    if(objetivo(mejor_pob) < c_solucion) {
                        memcpy(solucion, mejor_pob, (n_ciudades - 1) * sizeof(unsigned int));
                        c_solucion = objetivo(mejor_pob);
                        pob_mejor_sol = pob_generadas;
                    }

                    pob_generadas++;
                }

                printf("\nMEJOR SOLUCION: \n");
                printf("RECORRIDO: ");
                for(i = 0; i < n_ciudades - 1; i++) {
                    printf("%u ", solucion[i]);
                }
                printf("\nFUNCION OBJETIVO (km): %u\n", c_solucion);
                printf("ITERACION: %u\n", pob_mejor_sol);
            }
        }
        destruye_poblacion(&poblacion_prima);
        destruye_poblacion(&poblacion_inicial);

        fclose(archivo_aleatorios);
    }
    else {
        printf("ERROR ABRIENDO ARCHIVO DE ALEATORIOS %s.\n", traza);
    }

    return c_solucion;
}
