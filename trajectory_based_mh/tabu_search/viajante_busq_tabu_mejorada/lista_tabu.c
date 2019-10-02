
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    unsigned int nueva_pos_i;
    unsigned int nueva_pos_j;
} INTERCAMBIO;


typedef struct {
    unsigned int indice_ciudad;
    unsigned int indice_ins;
} INSERCION;


typedef struct celda {
    struct celda *siguiente;
    INSERCION elemento;
} CELDA;

typedef struct {
    unsigned int capacidad; //igual a la tenencia
    unsigned int tam; //numero de elementos en la lista
    CELDA *inicio; //puntero a la celda inicial, vacia: simplemente sirve como referencia para la celda del primer elemento
    CELDA *pos_insercion; //puntero a la celda anterior a la celda donde se tiene que insertar el elemento
} LISTA_CIRCULAR;

typedef LISTA_CIRCULAR * lista_tabu;

void crear_lista_tabu(lista_tabu *lt, unsigned int capacidad) {

    if(*lt == NULL && capacidad > 0) {
        *lt = (LISTA_CIRCULAR *)calloc(1, sizeof(LISTA_CIRCULAR));

        if(*lt != NULL) {
            (*lt)->capacidad = capacidad;
            (*lt)->inicio = (CELDA *)calloc(1, sizeof(CELDA));

            if((*lt)->inicio != NULL) {
                CELDA* aux = (*lt)->inicio;
                int i = 0;
                while(i < capacidad) {
                    aux->siguiente = (CELDA *)calloc(1, sizeof(CELDA));
                    aux = aux->siguiente;
                    if(aux == NULL) {
                        //error al reservar memoria
                        CELDA *aux_1;
                        aux = (*lt)->inicio;
                        while(aux != NULL) {
                            aux_1 = aux;
                            aux = aux->siguiente;
                            free(aux_1);
                        }
                        free(*lt);
                        *lt = NULL;
                        break;
                    }

                    i++;
                }
                if(i == capacidad) {
                    (*lt)->pos_insercion = (*lt)->inicio;
                    (*lt)->tam = 0;
                }
            }
            else {
                free(*lt);
                *lt = NULL;
            }
        }
    }
}

void destruir_lista_tabu(lista_tabu *lt) {

    if(*lt != NULL) {
        CELDA *celda, *aux;

        celda = (*lt)->inicio;
        int i = 0;
        //printf("Liberando lista tabú...\n");
        while(celda->siguiente != NULL && i < (*lt)->capacidad) {
            aux = celda;
            celda = celda->siguiente;
            free(aux);

            i++;
        }
        free((*lt)->inicio);
        free(*lt);
        *lt = NULL;
    }
}

void insertar_tabu(lista_tabu lt, INSERCION in) {

    if(lt != NULL) {
        CELDA *nueva_celda = lt->pos_insercion->siguiente;
        if(nueva_celda == NULL) { //se alcanzo el final de la lista: insertar al principio
            nueva_celda = lt->inicio->siguiente;
        }
        nueva_celda->elemento = in;
        //printf("Insertando en lista tabú: (%u, %u)\n", nueva_celda->elemento.indice_ciudad, nueva_celda->elemento.indice_ins);

        lt->pos_insercion = nueva_celda;
        lt->tam = (lt->tam < lt->capacidad) ? lt->tam + 1 : lt->capacidad;
    }
}

int es_tabu(lista_tabu lt, INSERCION in) {
    int encontrado = 0;

    if(lt != NULL) {
        CELDA *ce;
        INSERCION ele;

        int i = 0;
        ce = lt->inicio;
        while(i < lt->tam && !encontrado) {
            ce = ce->siguiente;
            ele = ce->elemento;
            if(ele.indice_ciudad == in.indice_ciudad && ele.indice_ins == in.indice_ins)  {
                encontrado = 1;
                //printf("Encontrado!\n");
            }
            i++;
        }
    }

    return encontrado;
}

void vaciar_lista_tabu(lista_tabu lt) {
    lt->tam = 0;
    lt->pos_insercion = lt->inicio;
}

void imprimir_lista_tabu(lista_tabu lt) {

    if(lt != NULL) {
        int i;
        CELDA *ce;
        INSERCION ele;

        i = 0;
        printf("\tLISTA TABU:\n");
        //del mas antiguo al mas reciente
        if(lt->tam == lt->capacidad) {
            //lista tabu llena: el mas antiguo es el siguiente al ultimo insertado (pos_insercion)
            ce = lt->pos_insercion;
            while(ce->siguiente != NULL && i < lt->tam) {
                ce = ce->siguiente;
                ele = ce->elemento;
                printf("\t%u %u\n", ele.indice_ciudad, ele.indice_ins);
                i++;
            }
        }
        ce = lt->inicio;
        while(ce->siguiente != NULL && i < lt->tam) {
            ce = ce->siguiente;
            ele = ce->elemento;
            printf("\t%u %u\n", ele.indice_ciudad, ele.indice_ins);
            i++;
        }

        //printf("Tabúes: %d\n", i);
    }
}
