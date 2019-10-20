#include <stdio.h>
#include <stdlib.h>

typedef struct {
    unsigned int new_pos_i;
    unsigned int new_pos_j;
} PERMUTATION;


typedef struct {
    unsigned int city_index;
    unsigned int insertion_index;
} INSERTION;


typedef struct cell {
    struct cell *next;
    INSERTION element;
} CELL;

typedef struct {
    unsigned int capacity; // maximum size
    unsigned int size; // number of elements in the list
    CELL *start;
    CELL *insertion_pos; // points to the cell that is previous to the one the insertion is targeted
} CIRCULAR_LIST;

typedef CIRCULAR_LIST * tabu_list;

void create_tabu_list(tabu_list *lt, unsigned int capacity) {

    if(*lt == NULL && capacity > 0) {
        *lt = (CIRCULAR_LIST *)calloc(1, sizeof(CIRCULAR_LIST));

        if(*lt != NULL) {
            (*lt)->capacity = capacity;
            (*lt)->start = (CELL *)calloc(1, sizeof(CELL));

            if((*lt)->start != NULL) {
                CELL* aux = (*lt)->start;
                int i = 0;
                while(i < capacity) {
                    aux->next = (CELL *)calloc(1, sizeof(CELL));
                    aux = aux->next;
                    if(aux == NULL) {
                        // memory allocation error
                        CELL *aux_1;
                        aux = (*lt)->start;
                        while(aux != NULL) {
                            aux_1 = aux;
                            aux = aux->next;
                            free(aux_1);
                        }
                        free(*lt);
                        *lt = NULL;
                        break;
                    }

                    i++;
                }
                if(i == capacity) {
                    (*lt)->insertion_pos = (*lt)->start;
                    (*lt)->size = 0;
                }
            }
            else {
                free(*lt);
                *lt = NULL;
            }
        }
    }
}

void free_tabu_list(tabu_list *lt) {

    if(*lt != NULL) {
        CELL *cell, *aux;

        cell = (*lt)->start;
        int i = 0;
        while(cell->next != NULL && i < (*lt)->capacity) {
            aux = cell;
            cell = cell->next;
            free(aux);

            i++;
        }
        free((*lt)->start);
        free(*lt);
        *lt = NULL;
    }
}

void insert_tabu(tabu_list lt, INSERTION in) {

    if(lt != NULL) {
        CELL *new_cell = lt->insertion_pos->next;
        if(new_cell == NULL) { // end of the list: insert at the start
            new_cell = lt->start->next;
        }
        new_cell->element = in;

        lt->insertion_pos = new_cell;
        lt->size = (lt->size < lt->capacity) ? lt->size + 1 : lt->capacity;
    }
}

int is_tabu(tabu_list lt, INSERTION in) {
    int found = 0;

    if(lt != NULL) {
        CELL *ce;
        INSERTION ele;

        int i = 0;
        ce = lt->start;
        while(i < lt->size && !found) {
            ce = ce->next;
            ele = ce->element;
            if(ele.city_index == in.city_index && ele.insertion_index == in.insertion_index)  {
                found = 1;
            }
            i++;
        }
    }

    return found;
}

void empty_tabu_list(tabu_list lt) {
    lt->size = 0;
    lt->insertion_pos = lt->start;
}

void print_tabu_list(tabu_list lt) {

    if(lt != NULL) {
        int i;
        CELL *ce;
        INSERTION ele;

        i = 0;
        printf("\tTABU LIST:\n");
        // from oldest to newest
        if(lt->size == lt->capacity) {
            // tabu list is full: oldest is next to last inserted (insertion_pos)
            ce = lt->insertion_pos;
            while(ce->next != NULL && i < lt->size) {
                ce = ce->next;
                ele = ce->element;
                printf("\t%u %u\n", ele.city_index, ele.insertion_index);
                i++;
            }
        }
        ce = lt->start;
        while(ce->next != NULL && i < lt->size) {
            ce = ce->next;
            ele = ce->element;
            printf("\t%u %u\n", ele.city_index, ele.insertion_index);
            i++;
        }
    }
}
