typedef struct {
    unsigned int nueva_pos_i;
    unsigned int nueva_pos_j;
} INTERCAMBIO;

typedef struct {
    unsigned int indice_ciudad;
    unsigned int indice_ins;
} INSERCION;

typedef void * lista_tabu;

void crear_lista_tabu(lista_tabu *lt, unsigned int capacidad);
void destruir_lista_tabu(lista_tabu *lt);
//void insertar_tabu(lista_tabu lt, INTERCAMBIO in);
//int es_tabu(lista_tabu lt, INTERCAMBIO in); //devuelve 0 si no esta en la lista tabu
void insertar_tabu(lista_tabu lt, INSERCION in);
int es_tabu(lista_tabu lt, INSERCION in); //devuelve 0 si no esta en la lista tabu
void vaciar_lista_tabu(lista_tabu lt);
void imprimir_lista_tabu(lista_tabu lt);
