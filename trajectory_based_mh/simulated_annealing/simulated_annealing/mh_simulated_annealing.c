#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tsp.h"

#define PHI 0.5
#define MU 0.01
#define MAX_NEIGHBOURS 80
#define MAX_SUCCESSES 20
#define ITE_STOP 10000

SOLUTION current_solution = NULL, s_prime = NULL, best_neighbour = NULL;
unsigned int neighbour_fitness = -1, c_best_neighbour = -1;
unsigned int coolings = 0;
int delta = 0;
double initial_temperature = 0.0, temperature = 0.0, exponential = 0.0, random_value = 0.0;
unsigned int tested_candidates = 0, accepted_candidates = 0;

/******SIMULATED ANNEALING******/
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

void generate_candidate_solution() {
    int city_index, insertion_index, best_insertion_index;
    unsigned int city;
    int i;

    city_index = floor(drand48() * (n_cities - 1));
    city = current_solution[city_index];
    printf("\tCITY INDEX: %d\n", city_index);
    printf("\tCITY: %u\n", city);

    // initially, best neighbour is the first one
    neighbour_fitness = 0;
    insertion_index = (city_index + 1) % (n_cities - 1);
    insertion(city_index, insertion_index);
    for(i = 0; i < n_cities - 1; i++) {
        best_neighbour[i] = s_prime[i];
    }
    c_best_neighbour = neighbour_fitness;
    best_insertion_index = insertion_index;
    insertion_index = (insertion_index + 1) % (n_cities - 1);
    // search best of all
    do {
        insertion(city_index, insertion_index);
        if(neighbour_fitness < c_best_neighbour || (neighbour_fitness == c_best_neighbour && insertion_index < best_insertion_index)) {
            for(i = 0; i < n_cities - 1; i++) {
                best_neighbour[i] = s_prime[i];
            }
            c_best_neighbour = neighbour_fitness;
            best_insertion_index = insertion_index;
        }

        insertion_index = (insertion_index + 1) % (n_cities - 1);
    } while(insertion_index != city_index);
    printf("\tINSERTION INDEX: %u\n", best_insertion_index);
    printf("\tWALK:");
    for(i = 0; i < n_cities - 1; i++) {
        printf(" %u", best_neighbour[i]);
    }
    printf(" \n");
    printf("\tFITNESS (km): %u\n", c_best_neighbour);
}

void cool(int k) {
    temperature = initial_temperature / (1 + k);
    coolings++;
    tested_candidates = 0;
    accepted_candidates = 0;
    printf("\n============================\nCOOLING: %u\n============================\n", coolings);
    printf("TEMPERATURE: %.6lf\n", temperature);
}

unsigned int simulated_annealing() {
    unsigned int solution_fitness = 0;

    current_solution = (unsigned int *)calloc(n_cities - 1, sizeof(unsigned int));
    s_prime = (unsigned int *)calloc(n_cities - 1, sizeof(unsigned int));
    best_neighbour = (unsigned int *)calloc(n_cities - 1, sizeof(unsigned int));
    neighbour_fitness = -1;
    c_best_neighbour = -1;
    coolings = 0;
    tested_candidates = 0, accepted_candidates = 0;
    if(current_solution != NULL && best_neighbour != NULL && s_prime != NULL) {
        int i = 0, k = 1, ite_solution = 0;

        srand48((unsigned)time(NULL));
        generate_initial_solution();
        solution_fitness = fitness(solution);
        printf("INITIAL SOLUTION:\n");
        printf("\tWALK:");
        for(i = 0; i < n_cities - 1; i++) {
            printf(" %u", solution[i]);
        }
        printf(" \n");
        for(i = 0; i < n_cities - 1; i++) { // first current solution: initial solution
            current_solution[i] = solution[i];
        }
        printf("\tFITNESS (km): %u\n", solution_fitness);
        initial_temperature = MU/-log(PHI) * solution_fitness;
        temperature = initial_temperature;
        printf("\tINITIAL TEMPERATURE: %.6lf\n", initial_temperature);
        do {
            printf("\nITERATION: %d\n", k);
            generate_candidate_solution();
            delta = c_best_neighbour - fitness(current_solution);
            printf("\tDELTA: %d\n", delta);
            printf("\tTEMPERATURE: %.6lf\n", temperature);
            exponential = exp(-delta/temperature);
            printf("\tEXPONENTIAL VALUE: %.6lf\n", exponential);
            random_value = drand48();
            if(delta < 0 || random_value < exponential) { // accepted
                for(i = 0; i < n_cities - 1; i++) {
                    current_solution[i] = best_neighbour[i];
                }
                accepted_candidates++;
                tested_candidates++;
                printf("\tCANDIDATE SOLUTION ACCEPTED\n");

                if(c_best_neighbour < solution_fitness) { // global optimum
                    for(i = 0; i < n_cities - 1; i++) {
                        solution[i] = current_solution[i];
                    }
                    solution_fitness = c_best_neighbour;
                    ite_solution = k;
                }
            }
            else { // not accepted
                tested_candidates++;
            }
            printf("\tTESTED CANDIDATES: %u, ACCEPTED: %u\n", tested_candidates, accepted_candidates);
            if(tested_candidates == MAX_NEIGHBOURS || accepted_candidates == MAX_SUCCESSES) { // cooling
                cool(coolings + 1);
            }

            k++;
        } while(k <= ITE_STOP);

        printf("\n\nBEST SOLUTION: \n");
        printf("\tWALK:");
        for(i = 0; i < n_cities - 1; i++) {
            printf(" %u", solution[i]);
        }
        printf(" \n");
        printf("\tFITNESS (km): %u\n", solution_fitness);
        printf("\tITERATION: %d\n", ite_solution);
        printf("\tmu = %lf, phi = %lf\n", MU, PHI);
        printf("COOLINGS: %u\n", coolings);
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

    return solution_fitness;
}


/******SIMULATED ANNEALING FROM FILE******/
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

void generate_candidate_solution_file() {
    double r = 0.0;
    int city_index, insertion_index, best_insertion_index;
    unsigned int city;
    int i;

    fscanf(random_file, "%lf\r\n", &r);
    city_index = floor(r * (n_cities - 1));
    city = current_solution[city_index];
    printf("\tCITY INDEX: %d\n", city_index);
    printf("\tCITY: %u\n", city);

    // initially, best neighbour is the first one
    neighbour_fitness = 0;
    insertion_index = (city_index + 1) % (n_cities - 1);
    insertion(city_index, insertion_index);
    for(i = 0; i < n_cities - 1; i++) {
        best_neighbour[i] = s_prime[i];
    }
    c_best_neighbour = neighbour_fitness;
    best_insertion_index = insertion_index;
    insertion_index = (insertion_index + 1) % (n_cities - 1);
    // search best neighbour
    do {
        insertion(city_index, insertion_index);
        if(neighbour_fitness < c_best_neighbour || (neighbour_fitness == c_best_neighbour && insertion_index < best_insertion_index)) {
            for(i = 0; i < n_cities - 1; i++) {
                best_neighbour[i] = s_prime[i];
            }
            c_best_neighbour = neighbour_fitness;
            best_insertion_index = insertion_index;
        }

        insertion_index = (insertion_index + 1) % (n_cities - 1);
    } while(insertion_index != city_index);
    printf("\tINSERTION INDEX: %u\n", best_insertion_index);
    printf("\tWALK:");
    for(i = 0; i < n_cities - 1; i++) {
        printf(" %u", best_neighbour[i]);
    }
    printf(" \n");
    printf("\tFITNESS (km): %u\n", c_best_neighbour);
}

unsigned int simulated_annealing_file(char* trace) {
    unsigned int solution_fitness = 0;

    random_file = fopen(trace, "r");
    if(random_file != NULL) {
        current_solution = (unsigned int *)calloc(n_cities - 1, sizeof(unsigned int));
        s_prime = (unsigned int *)calloc(n_cities - 1, sizeof(unsigned int));
        best_neighbour = (unsigned int *)calloc(n_cities - 1, sizeof(unsigned int));
        neighbour_fitness = -1;
        c_best_neighbour = -1;
        coolings = 0;
        tested_candidates = 0, accepted_candidates = 0;
        if(current_solution != NULL && best_neighbour != NULL && s_prime != NULL) {
            int i = 0, k = 1, ite_solution = 0;

            srand48((unsigned)time(NULL));
            generate_initial_solution_file();
            solution_fitness = fitness(solution);
            printf("INITIAL SOLUTION:\n");
            printf("\tWALK:");
            for(i = 0; i < n_cities - 1; i++) {
                printf(" %u", solution[i]);
            }
            printf(" \n");
            for(i = 0; i < n_cities - 1; i++) { // first current solution: initial solution
                current_solution[i] = solution[i];
            }
            printf("\tFITNESS (km): %u\n", solution_fitness);
            initial_temperature = MU/-log(PHI) * solution_fitness;
            temperature = initial_temperature;
            printf("\tINITIAL TEMPERATURE: %.6lf\n", initial_temperature);
            do {
                printf("\nITERATION: %d\n", k);
                generate_candidate_solution_file();
                delta = c_best_neighbour - fitness(current_solution);
                printf("\tDELTA: %d\n", delta);
                printf("\tTEMPERATURE: %.6lf\n", temperature);
                exponential = exp(-delta/temperature);
                printf("\tEXPONENTIAL VALUE: %.6lf\n", exponential);
                fscanf(random_file, "%lf\r\n",&random_value);
                if(delta < 0 || random_value < exponential) { // accepted
                    for(i = 0; i < n_cities - 1; i++) {
                        current_solution[i] = best_neighbour[i];
                    }
                    accepted_candidates++;
                    tested_candidates++;
                    printf("\tCANDIDATE SOLUTION ACCEPTED\n");

                    if(c_best_neighbour < solution_fitness) { // global optimum
                        for(i = 0; i < n_cities - 1; i++) {
                            solution[i] = current_solution[i];
                        }
                        solution_fitness = c_best_neighbour;
                        ite_solution = k;
                    }
                }
                else { // not accepted
                    tested_candidates++;
                }
                printf("\tTESTED CANDIDATES: %u, ACCEPTED: %u\n", tested_candidates, accepted_candidates);
                if(tested_candidates == MAX_NEIGHBOURS || accepted_candidates == MAX_SUCCESSES) { // cooling
                    cool(coolings + 1);
                }

                k++;
            } while(k <= ITE_STOP);

            printf("\n\nBEST SOLUTION: \n");
            printf("\tWALK:");
            for(i = 0; i < n_cities - 1; i++) {
                printf(" %u", solution[i]);
            }
            printf(" \n");
            printf("\tFITNESS (km): %u\n", solution_fitness);
            printf("\tITERATION: %d\n", ite_solution);
            printf("\tmu = %lf, phi = %lf\n", MU, PHI);
            printf("COOLINGS: %u\n", coolings);
        }

        fclose(random_file);
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
    }

    return solution_fitness;
}
