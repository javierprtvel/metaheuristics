#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "tsp.h"

typedef unsigned char** PERMUTATIONS; // binary flag matrix that marks generated permutations (lower triangular)

PERMUTATIONS p;
SOLUTION s_prime = NULL, best_neighbour = NULL;
unsigned char any_neighbours_left = (unsigned char) 0;

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

/******LOCAL SEARCH******/
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

    printf("Initial solution: ");
    for(i = 0; i < n_cities - 2; i++) {
        printf("%u-", solution[i]);
    }
    printf("%u\n\n", solution[i]);
}

void generate_neighbour() {
    int i = 0, j = 0;
    unsigned int tmp = -1;

    // current solution permutation
    i = floor(drand48() * (n_cities - 2));
    j = floor(drand48() * (n_cities - 2));
    if(i < j) {
        tmp = i;
        i = j;
        j = tmp;
    }


    int i_initial = i, j_initial = j;
    while(p[i][j] && any_neighbours_left) { // already generated permutation
        j = (j + 1) % (i + 1);
        i = (j == 0)? (i + 1) % (n_cities - 2) : i;

        if(i == i_initial && j == j_initial) { // back to the start -> generated all neighbourhood
            any_neighbours_left = (unsigned char) 0;
        }
    }
    if(any_neighbours_left) {
        int k = 0;
        for(; k < n_cities - 1; k++) {
            s_prime[k] = solution[k];
        }
        tmp = s_prime[i + 1];
        s_prime[i + 1] = s_prime[j];
        s_prime[j] = tmp;

        p[i][j] = (unsigned char) 1;
        printf("Permutation (%u, %u)\n", i, j);
    }
}

unsigned int local_search() {
    unsigned int solution_fitness = 0;

    srand48((unsigned)time(NULL));
    generate_initial_solution();
    fitness(solution);

    p = NULL;
    initialize_permutations();
    any_neighbours_left = (unsigned char) 1;
    best_neighbour = (unsigned int *)calloc(n_cities - 1, sizeof(unsigned int));
    s_prime = (unsigned int *)calloc(n_cities - 1, sizeof(unsigned int));
    if(p != NULL && best_neighbour != NULL && s_prime != NULL) {
        int i = 0, j = 0;

        for(; i < n_cities - 1; i++) { // initially, first better is the current solution
            best_neighbour[i] = solution[i];
        }
        do {
            for(i = 0; i < n_cities - 1; i++) {
                solution[i] = best_neighbour[i]; // current solution: first better of previous iteration
            }

            do {
                generate_neighbour();
            } while(fitness(s_prime) >= fitness(solution) && any_neighbours_left);

            if(fitness(s_prime) < fitness(solution)) {
                for(i = 0; i < n_cities - 1; i++) {
                    best_neighbour[i] = s_prime[i];
                }
                solution_fitness = fitness(best_neighbour);

                printf("First better neighbour found. Distance: %u\n", solution_fitness);
                printf("Optimal walk: ");
                for(j = 0; j < n_cities - 2; j++) {
                    printf("%u-", best_neighbour[j]);
                }
                printf("%u\n\n", best_neighbour[j]);

                reset_permutations();
                any_neighbours_left = (unsigned char) 1;
            }
            else {
                printf("Generated all neighbourhood without success\n");
            }
        } while(fitness(best_neighbour) < fitness(solution));
        solution_fitness = fitness(solution);
        printf("\nBEST SOLUTION: \n");
        printf("WALK: ");
        for(i = 0; i < n_cities - 1; i++) {
            printf("%u ", solution[i]);
        }
        printf("\nFITNESS (km): %u\n", solution_fitness);
    }

    if(best_neighbour != NULL) {
        free(best_neighbour);
        best_neighbour = NULL;
    }
    if(s_prime != NULL) {
        free(s_prime);
        s_prime = NULL;
    }
    free_permutations();

    return solution_fitness;
}


/******LOCAL SEARCH FROM FILE******/
FILE *random_file = NULL;
unsigned int n_solution = 0, n_neighbour = 0;

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

void generate_neighbour_file() {
    int i = 0, j = 0;
    unsigned int tmp = -1;
    double r = 0.0;

    fscanf(random_file, "%lf\r\n", &r);

    i = floor(r * (n_cities - 1));
    fscanf(random_file, "%lf\r\n", &r);

    j = floor(r * (n_cities - 1));

    if(i < j) {
        tmp = i;
        i = j;
        j = tmp;
    }
    // index pair repetition
    while(i == j) {
        i = (i + 1) % (n_cities - 1);
        j = (i == 0)? 0 : (j + 1) % i;
    }
    i--; // permutation matrix does not have diagonal


    // permutation of current solution
    int i_initial = i, j_initial = j;
    while(p[i][j] && any_neighbours_left) { // already generated permutation
        j = (j + 1) % (i + 1);
        i = (j == 0)? (i + 1) % (n_cities - 2) : i;

        if(i == i_initial && j == j_initial) { // back to the start -> generated all neighbourhood
            any_neighbours_left = (unsigned char) 0;
        }
    }
    if(any_neighbours_left) {
        int k = 0;
        for(; k < n_cities - 1; k++) {
            s_prime[k] = solution[k];
        }
        tmp = s_prime[i + 1];
        s_prime[i + 1] = s_prime[j];
        s_prime[j] = tmp;

        printf("\tNEIGHBOUR V_%u -> Permutation: (%u, %u); [", n_neighbour, i + 1, j);
        for(k = 0; k < n_cities - 2; k++) {
            printf("%u, ", s_prime[k]);
        }
        printf("%u]; ", s_prime[n_cities - 2]);
        printf("%ukm\n", fitness(s_prime));

        p[i][j] = (unsigned char) 1;
        n_neighbour++;
    }
}

unsigned int local_search_file(char* trace) {
    unsigned int solution_fitness = 0;

    random_file = NULL;
    n_solution = 0;
    n_neighbour = 0;

    random_file = fopen(trace, "r");
    if(random_file != NULL) {
        generate_initial_solution_file();

        p = NULL;
        initialize_permutations();
        any_neighbours_left = (unsigned char) 1;
        best_neighbour = (unsigned int *)calloc(n_cities - 1, sizeof(unsigned int));
        s_prime = (unsigned int *)calloc(n_cities - 1, sizeof(unsigned int));
        if(p != NULL && best_neighbour != NULL && s_prime != NULL) {
            int i = 0;

            for(; i < n_cities - 1; i++) { //initially, first better is the current solution
                best_neighbour[i] = solution[i];
            }
            do {
                printf("\nSOLUTION S_%u -> [", n_solution);
                for(i = 0; i < n_cities - 2; i++) {
                    solution[i] = best_neighbour[i]; // current solution: first better of previous iteration
                    printf("%u, ", solution[i]);
                }
                solution[n_cities - 2] = best_neighbour[n_cities - 2];
                printf("%u]; ", solution[n_cities - 2]);
                printf("%ukm\n", fitness(solution));
                n_solution++;

                do {
                    generate_neighbour_file();
                } while(fitness(s_prime) >= fitness(solution) && any_neighbours_left);

                if(fitness(s_prime) < fitness(solution)) {
                    for(i = 0; i < n_cities - 1; i++) {
                        best_neighbour[i] = s_prime[i];
                    }
                    solution_fitness = fitness(best_neighbour);

                    reset_permutations();
                    any_neighbours_left = (unsigned char) 1;
                    n_neighbour = 0;
                }
            } while(fitness(best_neighbour) < fitness(solution));
            solution_fitness = fitness(solution);

            printf("\nBEST SOLUTION: \n");
            printf("WALK: ");
            for(i = 0; i < n_cities - 1; i++) {
                printf("%u ", best_neighbour[i]);
            }
            printf("\nFITNESS (km): %u\n", solution_fitness);
        }

        if(best_neighbour != NULL) {
            free(best_neighbour);
            best_neighbour = NULL;
        }
        if(s_prime != NULL) {
            free(s_prime);
            s_prime = NULL;
        }
        free_permutations();

        fclose(random_file);
        random_file = NULL;
    }

    return solution_fitness;
}
