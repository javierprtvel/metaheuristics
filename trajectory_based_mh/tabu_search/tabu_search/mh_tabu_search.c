#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tabu_list.h"
#include "tsp.h"

#define ITE_REINITIALIZATION 100
#define ITE_STOP 10000

typedef unsigned char** PERMUTATIONS; // binary flag matrix that marks generated permutations (lower triangular)

PERMUTATIONS p;
tabu_list lt = NULL;
SOLUTION current_solution = NULL, s_prime = NULL, best_neighbour = NULL;
int next_neighbour_i = 0, next_neighbour_j = 0;
unsigned int city = -1;
unsigned char any_neighbours_left = (unsigned char) 0;
unsigned int neighbour_fitness = -1, best_neighbour_fitness = -1;
INSERTION insertion_prime, best_neighbour_insertion;
unsigned int reset_count = 0;

void initialize_permutations() {
    int i = 0, j = 0;

    p = (unsigned char **)calloc(n_cities - 2, sizeof(unsigned char*));
    for(; i < n_cities - 2; i++) {
        p[i] = (unsigned char *)calloc(i + 1, sizeof(unsigned char));
        for(j = 0; j < i + 1; j++) {
            p[i][j] = (unsigned char) 0;
         }
    }
}

void reset_permutations() {
    int i = 0, j = 0;

    for(; i < n_cities - 2; i++) {
        for(j = 0; j < i + 1; j++) {
            p[i][j] = (unsigned char) 0;
        }
    }
}

void free_permutations() {

    if(p != NULL) {
        int i = 0;
        for(; i < n_cities - 2; i++) {
            if(p[i] != NULL) {
                free(p[i]);
                p[i] = NULL;
            }
        }
        free(p);
        p = NULL;
    }

}

/******TABU SEARCH******/
void generate_initial_solution() {
    int i = 0, j = 0;

    for(; i < n_cities - 1; i++) {
        solution[i] = 1 + floor(drand48() * (n_cities - 1));

        for(j = 0; j < i; j++) {
            if(solution[i] == solution[j]) {
                int repeated = 0;
                do {
                    repeated = 0;
                    solution[i] = solution[i] % (n_cities - 1) + 1;
                    for(j = 0; j < i; j++) {
                        if(solution[i] == solution[j]) {
                            repeated = 1;
                            break;
                        }
                    }
                } while(repeated);
                break;
            }
        }
    }
}

int aspiration_criteria_compliance(SOLUTION s) {

    if(fitness(s) < fitness(solution)) {
        return 1;
    }
    else {
        return 0;
    }
}

void insertion(int city_index, int insertion_index) {
    int i, j;
    unsigned int insertion_value;

    insertion_value = current_solution[city_index];
    if(insertion_index < city_index) {
        for(i = 0; i < insertion_index; i++) {
            s_prime[i] = current_solution[i];
        }
        s_prime[insertion_index] = insertion_value;
        for(i = insertion_index, j = insertion_index + 1; i < city_index; i++, j++) {
            s_prime[j] = current_solution[i];
        }
        for(i = city_index + 1; i < n_cities - 1; i++) {
            s_prime[i] = current_solution[i];
        }
    }
    else if(insertion_index > city_index) {
        for(i = 0; i < city_index; i++) {
            s_prime[i] = current_solution[i];
        }
        for(i = city_index + 1, j = city_index; i <= insertion_index; i++, j++) {
            s_prime[j] = current_solution[i];
        }
        s_prime[insertion_index] = insertion_value;
        for(i = insertion_index + 1; i < n_cities - 1; i++) {
            s_prime[i] = current_solution[i];
        }
    }

    neighbour_fitness = fitness(s_prime);
}

void generate_neighbour() {
    int i = 0, j = 0;
    
    // assign new neighbour to s_prime
    insertion(insertion_prime.city_index, insertion_prime.insertion_index);
    if(aspiration_criteria_compliance(s_prime) || !is_tabu(lt, insertion_prime)) {
        neighbour_fitness = fitness(s_prime);
    }
    else {
        neighbour_fitness = 0;
    }

    p[i][j] = (unsigned char) 1;
    next_neighbour_j = (j + 1) % (i + 1);
    next_neighbour_i = (next_neighbour_j == 0)? i + 1 : i;
}


unsigned int tabu_search() {
    unsigned int solution_fitness = 0;

    unsigned int city_index = -1;
    unsigned int insertion_index = -1, best_insertion_index = -1;
    p = NULL;
    initialize_permutations();
    any_neighbours_left = (unsigned char) 1;
    lt = NULL;
    create_tabu_list(&lt, n_cities);
    current_solution = (unsigned int *)calloc(n_cities - 1, sizeof(unsigned int));
    s_prime = (unsigned int *)calloc(n_cities - 1, sizeof(unsigned int));
    best_neighbour = (unsigned int *)calloc(n_cities - 1, sizeof(unsigned int));
    neighbour_fitness = -1;
    best_neighbour_fitness = -1;
    reset_count = 0;
    if(p != NULL && lt != NULL && current_solution != NULL && best_neighbour != NULL && s_prime != NULL) {
        int i = 0, j = 1, ite_without_improvement = 0, ite_solution = 0;

        srand48((unsigned)time(NULL));
        generate_initial_solution();
        solution_fitness = fitness(solution);
        printf("INITIAL WALK\n");
        printf("\tWALK:");
        for(i = 0; i < n_cities - 1; i++) {
            printf(" %u", solution[i]);
        }
        printf(" \n");
        printf("\tFITNESS (km): %u\n", solution_fitness);

        for(i = 0; i < n_cities - 1; i++) {
            current_solution[i] = solution[i]; // first current solution: initial solution
        }

        do {
            printf("\nITERATION: %d\n", j);
            
            city_index = floor(drand48() * (n_cities - 1));
            insertion_prime.city_index = city_index;
            city = current_solution[city_index];
            printf("\tCITY INDEX: %d\n", city_index);
            printf("\tCITY: %u\n", city);
            
            // initially, best neighbour is the first one
            neighbour_fitness = 0;
            insertion_index = (city_index + 1) % (n_cities - 1);
            insertion_prime.insertion_index = insertion_index;
            generate_neighbour();
            best_neighbour_fitness = neighbour_fitness;
            best_insertion_index = insertion_index;
            insertion_prime.insertion_index = best_insertion_index;
            memcpy(&best_neighbour_insertion, &insertion_prime, sizeof(INSERTION));
            insertion_index = (insertion_index + 1) % (n_cities - 1);
            // search best, non-tabu neighbour
            do {
                generate_neighbour();
                if(neighbour_fitness != 0 && (neighbour_fitness < best_neighbour_fitness || (neighbour_fitness == best_neighbour_fitness && insertion_index < best_insertion_index))) {
                    for(i = 0; i < n_cities - 1; i++) {
                        best_neighbour[i] = s_prime[i];
                    }
                    best_neighbour_fitness = neighbour_fitness;
                    memcpy(&best_neighbour_insertion, &insertion_prime, sizeof(INSERTION));
                }
                
                 insertion_index = (insertion_index + 1) % (n_cities - 1);
                 insertion_prime.insertion_index = insertion_index;
            } while(insertion_index != city_index);
            printf("\tINSERTION: (%u, %u)\n", best_neighbour_insertion.city_index, best_neighbour_insertion.insertion_index);
            // assign best to current solution and add insertion to tabu list
            printf("\tWALK:");
            for(i = 0; i < n_cities - 1; i++) {
                current_solution[i] = best_neighbour[i];
                printf(" %u", current_solution[i]);
            }
            printf(" \n");
            printf("\tFITNESS (km): %u\n", fitness(current_solution));
            insert_tabu(lt, best_neighbour_insertion);

            if(best_neighbour_fitness < solution_fitness) { // best neighbour is global optimum
                for(i = 0; i < n_cities - 1; i++) {
                    solution[i] = best_neighbour[i];
                }
                solution_fitness = fitness(solution);
                ite_solution = j;

                ite_without_improvement = 0;
            }
            else { // best neighbour does not improve current optimal solution
                ite_without_improvement++;
            }
            printf("\tITERATIONS WITHOUT IMPROVEMENT: %d\n", ite_without_improvement);
            print_tabu_list(lt);

            if(ite_without_improvement == ITE_REINITIALIZATION) {
                // reinitialization
                for(i = 0; i < n_cities - 1; i++) {
                    current_solution[i] = solution[i];
                }
                empty_tabu_list(lt);

                ite_without_improvement = 0;
                reset_count++;
                printf("\n***************\nREINITIALIZATION: %u\n***************\n", reset_count);
            }

            any_neighbours_left = (unsigned char) 1;
            next_neighbour_i = 0;
            next_neighbour_j = 0;
            j++;
        } while(j <= ITE_STOP);

        printf("\n\nBEST SOLUTION: \n");
        printf("\tWALK:");
        for(i = 0; i < n_cities - 1; i++) {
            printf(" %u", solution[i]);
        }
        printf(" \n");
        printf("\tFITNESS (km): %u\n", solution_fitness);
        printf("\tITERATION: %d\n", ite_solution);
    }

    if(best_neighbour != NULL) {
        free(best_neighbour);
        best_neighbour = NULL;
    }
    if(s_prime != NULL) {
        free(s_prime);
        s_prime = NULL;
    }
    if(current_solution != NULL) {
        free(current_solution);
        current_solution = NULL;
    }
    free_tabu_list(&lt);
    free_permutations();

    return solution_fitness;
}


/******TABU SEARCH FROM FILE******/
FILE *random_file = NULL;

void generate_initial_solution_file() {
    int i = 0, j = 0;
    double r = 0.0;

    for(; i < n_cities - 1; i++) {
        fscanf(random_file, "%lf\r\n", &r);
        solution[i] = 1 + floor(r * (n_cities - 1));

        for(j = 0; j < i; j++) {
            if(solution[i] == solution[j]) {
                int repeated = 0;
                do {
                    repeated = 0;
                    solution[i] = solution[i] % (n_cities - 1) + 1;
                    for(j = 0; j < i; j++) {
                        if(solution[i] == solution[j]) {
                            repeated = 1;
                            break;
                        }
                    }
                } while(repeated);
                break;
            }
        }
    }
}

unsigned int tabu_search_file(char* trace) {
    unsigned int solution_fitness = 0;

    random_file = fopen(trace, "r");
    if(random_file != NULL) {
        p = NULL;
        initialize_permutations();
        any_neighbours_left = (unsigned char) 1;
        lt = NULL;
        create_tabu_list(&lt, n_cities);
        current_solution = (unsigned int *)calloc(n_cities - 1, sizeof(unsigned int));
        s_prime = (unsigned int *)calloc(n_cities - 1, sizeof(unsigned int));
        best_neighbour = (unsigned int *)calloc(n_cities - 1, sizeof(unsigned int));
        neighbour_fitness = -1;
        best_neighbour_fitness = -1;
        reset_count = 0;
        if(p != NULL && lt != NULL && current_solution != NULL && best_neighbour != NULL && s_prime != NULL) {
            int i = 0, j = 1, ite_without_improvement = 0, ite_solution = 0;

            srand48((unsigned)time(NULL));
            generate_initial_solution_file();
            fclose(random_file);
            random_file = NULL;
            solution_fitness = fitness(solution);
            printf("INITIAL WALK\n");
            printf("\tWALK:");
            for(i = 0; i < n_cities - 1; i++) {
                printf(" %u", solution[i]);
            }
            printf(" \n");
            printf("\tFITNESS (km): %u\n", solution_fitness);

            for(i = 0; i < n_cities - 1; i++) {
                current_solution[i] = solution[i]; // first current solution: initial solution
            }

            do {
                printf("\nITERATION: %d\n", j);
                // initially, best neighbour is the first one
                neighbour_fitness = 0;
                do {
                    generate_neighbour();
                } while(neighbour_fitness == 0);
                for(i = 0; i < n_cities - 1; i++) {
                    best_neighbour[i] = s_prime[i];
                }
                best_neighbour_fitness = neighbour_fitness;
                memcpy(&best_neighbour_insertion, &insertion, sizeof(PERMUTATION));
                // search best, non-tabu neighbour
                do {
                    generate_neighbour();
                    if(neighbour_fitness != 0 && neighbour_fitness < best_neighbour_fitness) {
                        for(i = 0; i < n_cities - 1; i++) {
                            best_neighbour[i] = s_prime[i];
                        }
                        best_neighbour_fitness = neighbour_fitness;
                        memcpy(&best_neighbour_insertion, &insertion, sizeof(PERMUTATION));
                    }
                } while(any_neighbours_left);
                printf("\tPERMUTATION: (%u, %u)\n", best_neighbour_insertion.city_index, best_neighbour_insertion.insertion_index);

                // assign best to current solution and add insertion to tabu list
                printf("\tWALK:");
                for(i = 0; i < n_cities - 1; i++) {
                    current_solution[i] = best_neighbour[i];
                    printf(" %u", current_solution[i]);
                }
                printf(" \n");
                printf("\tFITNESS (km): %u\n", fitness(current_solution));
                insert_tabu(lt, best_neighbour_insertion);


                if(best_neighbour_fitness < solution_fitness) { // best neighbour is global optimum
                    for(i = 0; i < n_cities - 1; i++) {
                        solution[i] = best_neighbour[i];
                    }
                    solution_fitness = fitness(solution);
                    ite_solution = j;

                    ite_without_improvement = 0;
                }
                else { // best neighbour does not improve current optimal
                    ite_without_improvement++;
                }
                printf("\tITERATIONS WITHOUT IMPROVEMENT: %d\n", ite_without_improvement);
                print_tabu_list(lt);

                if(ite_without_improvement == ITE_REINITIALIZATION) {
                    // reinitialization
                    for(i = 0; i < n_cities - 1; i++) {
                        current_solution[i] = solution[i];
                    }
                    empty_tabu_list(lt);

                    ite_without_improvement = 0;
                    reset_count++;
                    printf("\n***************\nREINITIALIZATION: %u\n***************\n", reset_count);
                }

                reset_permutations();
                any_neighbours_left = (unsigned char) 1;
                next_neighbour_i = 0;
                next_neighbour_j = 0;
                j++;
            } while(j <= ITE_STOP);

            printf("\n\nBEST SOLUTION: \n");
            printf("\tWALK:");
            for(i = 0; i < n_cities - 1; i++) {
                printf(" %u", solution[i]);
            }
            printf(" \n");
            printf("\tFITNESS (km): %u\n", solution_fitness);
            printf("\tITERATION: %d\n", ite_solution);
        }

        if(best_neighbour != NULL) {
            free(best_neighbour);
            best_neighbour = NULL;
        }
        if(s_prime != NULL) {
            free(s_prime);
            s_prime = NULL;
        }
        if(current_solution != NULL) {
            free(current_solution);
            current_solution = NULL;
        }
        free_tabu_list(&lt);
        free_permutations();
    }


    return solution_fitness;
}
