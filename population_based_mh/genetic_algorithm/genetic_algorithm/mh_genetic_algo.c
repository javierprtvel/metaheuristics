#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tsp.h"

#define SORT_TYPE SOLUTION
#define SORT_NAME population
#define SORT_CMP(x, y) ((fitness(x)) < (fitness(y)) ? -1 : ((fitness(x)) == (fitness(y)) ? 0 : 1))
#include "sort.h"

#define POPULATION_SIZE 100
#define ELITISM 2
#define GENERATION_LIMIT 1000
#define CROSS_PROB 0.9
#define MUTA_PROB 0.01

typedef struct {
    SOLUTION genotypes[POPULATION_SIZE];
} POPULATION;

POPULATION initial_population, prime_population;
SOLUTION best[ELITISM];
unsigned int tournament = 0;

/******GENETIC ALGORITHM******/
void generate_random_solution(SOLUTION genotype) {
    int i = 0, j = 0;

    for(; i < n_cities - 1; i++) {
        genotype[i] = 1 + (int)(floor(drand48() * (n_cities - 1))) % (n_cities - 1);

        for(j = 0; j < i; j++) {
            if(genotype[i] == genotype[j]) {
                int repeated = 0;
                do {
                    repeated = 0;
                    genotype[i] = genotype[i] % (n_cities - 1) + 1;
                    for(j = 0; j < i; j++) {
                        if(genotype[i] == genotype[j]) {
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

void generate_greedy_solution(SOLUTION genotype) {
    int i = 0, j = 0, k = 0;
    int repeated = 0;
    unsigned int departure_city = 0, min_dist_city = 0, min_dist = 0;

    // first gene with random allele; following are calculated according to minimum distance
    genotype[0] = 1 + (int)(floor(drand48() * (n_cities - 1))) % (n_cities - 1);
    for(i = 1; i < n_cities - 1; i++) {
        departure_city = genotype[i - 1];

        // alleles already assigned are not taken into account (cities already in the walk)
        min_dist_city = 0;
        do {
            min_dist_city = min_dist_city % (n_cities - 1) + 1;
            repeated = 0;
            for(k = 0; k < i; k++) {
                if(min_dist_city == genotype[k]) {
                    repeated = 1;
                    break;
                }
            }
        } while(repeated);
        min_dist = (departure_city > min_dist_city)? d[departure_city - 1][min_dist_city]
            : d[min_dist_city - 1][departure_city];
        genotype[i] = min_dist_city;

        j = 1;
        while(j < (n_cities - i - 1)) {
            // alleles already assigned are not taken into account (cities already in the walk)
            do {
                repeated = 0;
                min_dist_city = min_dist_city % (n_cities - 1) + 1;
                for(k = 0; k < i; k++) {
                    if(min_dist_city == genotype[k]) {
                        repeated = 1;
                        break;
                    }
                }
            } while(repeated);

            if(departure_city > min_dist_city && d[departure_city - 1][min_dist_city] < min_dist) {
                genotype[i] = min_dist_city;
                min_dist = d[departure_city - 1][min_dist_city];
            }
            else if(departure_city < min_dist_city && d[min_dist_city - 1][departure_city] < min_dist) {
                genotype[i] = min_dist_city;
                min_dist = d[min_dist_city - 1][departure_city];
            }

            j++;
        }
    }
}

int generate_initial_population() {
    int success = 0;

    unsigned int solution_fitness = -1;
    int i = 0;
    // random genotypes (individuals)
    initial_population.genotypes[0] = (SOLUTION)calloc((n_cities - 1), sizeof(unsigned int));
    if(initial_population.genotypes[0] == NULL) {
        return success;
    }
    generate_random_solution(initial_population.genotypes[0]);
    solution_fitness = fitness(initial_population.genotypes[0]);
    for(i = 1; i < POPULATION_SIZE/2; i++) {
        initial_population.genotypes[i] = (SOLUTION)calloc((n_cities - 1), sizeof(unsigned int));
        if(initial_population.genotypes[i] == NULL) {
            break;
        }
        generate_random_solution(initial_population.genotypes[i]);

        if(fitness(initial_population.genotypes[i]) < solution_fitness) { // best solution of the population
            memcpy(solution, initial_population.genotypes[i], (n_cities - 1) * sizeof(unsigned int));
            solution_fitness = fitness(solution);
        }
    }
    // semi-random genotypes
    for(; i < POPULATION_SIZE; i++) {
        initial_population.genotypes[i] = (SOLUTION)calloc((n_cities - 1), sizeof(unsigned int));
        if(initial_population.genotypes[i] == NULL) {
            break;
        }
        generate_greedy_solution(initial_population.genotypes[i]);

        if(fitness(initial_population.genotypes[i]) < solution_fitness) { // best solution of the population
            memcpy(solution, initial_population.genotypes[i], (n_cities - 1) * sizeof(unsigned int));
            solution_fitness = fitness(solution);
        }
    }

    if(i == POPULATION_SIZE) {
        success = 1;
    }

    return success;
}

void free_population(POPULATION *pop) {
    int i = 0;

    for(; i < POPULATION_SIZE; i++) {
        if(pop->genotypes[i] != NULL) {
            free(pop->genotypes[i]);
            pop->genotypes[i] = NULL;
        }
    }
}

void print_population(POPULATION pop) {
    int i = 0, j = 0;

    for(; i < POPULATION_SIZE; i++) {
        printf("INDIVIDUAL %d = {FITNESS (km): %u, WALK: ", i, fitness(pop.genotypes[i]));
        for(j = 0; j < n_cities - 1; j++) {
            printf("%u ", pop.genotypes[i][j]);
        }
        printf("}\n");
    }
}

int select_father(unsigned int tournament) {
    int i = 0, j = 0;
    int father_index = 0;

    /* tournament of k=2 random genotypes (should be programmed with vectors and loops parametrized
     * with k for the algorithm to be generic
    */
    i = floor(drand48() * (POPULATION_SIZE - 1));
    j = floor(drand48() * (POPULATION_SIZE - 1));
    if(fitness(initial_population.genotypes[i]) <= fitness(initial_population.genotypes[j])) {
        father_index = i;
    }
    else {
        father_index = j;
    }
    printf("\tTOURNAMENT %u: %d %d %d wins\n", tournament, i, j, father_index);

    return father_index;
}

void crossing(int index_1, int index_2, int ite) {
    SOLUTION p1 = initial_population.genotypes[index_1];
    SOLUTION p2 = initial_population.genotypes[index_2];
    SOLUTION h1 = prime_population.genotypes[ite + ELITISM];
    SOLUTION h2 = prime_population.genotypes[ite + ELITISM + 1];
    double random = drand48();
    int pt_1 = 0, pt_2 = 0, tmp = 0;
    int n_cut_area = 0;
    int i = 0, j = 0, k1 = 0, k2 = 0, z = 0;
    int rep = 0;

    printf("\tCROSSING: (%d, %d) (RANDOM: %lf)\n", ite, ite + 1, random);
    printf("\t\tFATHER: = {FITNESS (km): %u, WALK: ", fitness(p1));
    for(i = 0; i < n_cities - 1; i++) {
        printf("%u ", p1[i]);
    }
    printf("}\n");
    printf("\t\tFATHER: = {FITNESS (km): %u, WALK: ", fitness(p2));
    for(i = 0; i < n_cities - 1; i++) {
        printf("%u ", p2[i]);
    }
    printf("}\n");

    if(random <= CROSS_PROB) {
        pt_1 = (int)floor(drand48() * (n_cities - 1)) % (n_cities - 1);
        pt_2 = (int)floor(drand48() * (n_cities - 1)) % (n_cities - 1);
        printf("\t\tCUTS: (%d, %d)\n", pt_1, pt_2);
        if(pt_1 > pt_2) {
            tmp = pt_1;
            pt_1 = pt_2;
            pt_2 = tmp;
        }
        n_cut_area = pt_2 - pt_1 + 1;

        memcpy(h1 + pt_1, p1 + pt_1, n_cut_area * sizeof(unsigned int));
        memcpy(h2 + pt_1, p2 + pt_1, n_cut_area * sizeof(unsigned int));
        i = 0;
        k1 = (pt_2 + 1) % (n_cities - 1), k2 = k1, z = k1;
        while(i < (n_cities - 1) - n_cut_area) {
            // p2-h1 inheritance
            do {
                rep = 0;
                for(j = pt_1; j <= pt_2; j++) {
                    if(p2[k1] == h1[j]) {
                        k1= (k1 + 1) % (n_cities - 1);
                        rep = 1;
                        break;
                    }
                }
            } while(rep);
            h1[z] = p2[k1];
            // p1-h2 inheritance
            do {
                rep = 0;
                for(j = pt_1; j <= pt_2; j++) {
                    if(p1[k2] == h2[j]) {
                        k2 = (k2 + 1) % (n_cities - 1);
                        rep = 1;
                        break;
                    }
                }
            } while(rep);
            h2[z] = p1[k2];

            z = (z + 1) % (n_cities - 1);
            k1 = (k1 + 1) % (n_cities - 1);
            k2 = (k2 + 1) % (n_cities - 1);
            i++;
        }

        printf("\t\tCHILD: = {FITNESS (km): %u, WALK: ", fitness(h1));
        for(i = 0; i < n_cities - 1; i++) {
            printf("%u ", h1[i]);
        }
        printf("}\n");
        printf("\t\tCHILD: = {FITNESS (km): %u, WALK: ", fitness(h2));
        for(i = 0; i < n_cities - 1; i++) {
            printf("%u ", h2[i]);
        }
        printf("}\n");
    }
    else { // children are identical to their fathers
        printf("\t\tNOT CROSSED\n");
        memcpy(h1, p1, (n_cities - 1) * sizeof(unsigned int));
        memcpy(h2, p2, (n_cities - 1) * sizeof(unsigned int));
    }
    printf("\n");
}

void permutation(SOLUTION *s, int i, int j) {
    unsigned int tmp;

    tmp = (*s)[i];
    (*s)[i] = (*s)[j];
    (*s)[j] = tmp;
}

void mutation(int index_child) {
    SOLUTION child = prime_population.genotypes[index_child];
    double random = 0.0;
    int pos_i = 0, pos_j = 0;
    int i = 0;

    printf("\tINDIVIDUAL %d\n", index_child - ELITISM);
    printf("\tPREVIOUS WALK: ");
    for(; i < n_cities - 1; i++) {
        printf("%u ", child[i]);
    }
    printf("\n");

    for(i = 0; i < n_cities - 1; i++) {
        random = drand48();
        if(random <= MUTA_PROB) {
            pos_i = i;
            pos_j = (int)floor(drand48() * (n_cities - 1)) % (n_cities - 1);
            printf("\t\tPOSITION: %d (RANDOM %lf) INTERCHANGED WITH: %d\n", i, random, pos_j);
            permutation(&child, pos_i, pos_j);
        }
        else {
            //no muta
            printf("\t\tPOSITION: %d (RANDOM %lf) NOT MUTATED\n", i, random);
        }
    }

    printf("\tNEW WALK: ");
    for(i = 0; i < n_cities - 1; i++) {
        printf("%u ", child[i]);
    }
    printf("\n");
    printf("\n");
}

void replacement() {
    /* Select k best genotypes of the population and insert them in decreasing order at the beginning
      of the population
    */
    SOLUTION best_solution = NULL, aux = NULL;
    unsigned int best_fitness = 0, aux_fitness;
    int i = 0, j = 0, k = 0;
    int rep = 0;
    memset(best, 0, ELITISM * sizeof(SOLUTION));
    for(i = 0; i < ELITISM; i++) {
        // first individuals already in best vector are ignored
        j = -1;
        do {
            j++;
            rep = 0;
            for(k = 0; k < ELITISM && !rep; k++) {
                if(initial_population.genotypes[j] == best[k]) { // same individual
                    rep = 1;
                }
            }
        } while(rep);
        best_solution = initial_population.genotypes[j];
        best_fitness = fitness(best_solution);

        // initial values set: calculate next minimum
        for(; j < POPULATION_SIZE; j++) {
            aux = initial_population.genotypes[j];
            aux_fitness = fitness(aux);
            rep = 0;
            for(k = 0; k < ELITISM && !rep; k++) {
                if(aux == best[k]) {
                    rep = 1;
                }
            }
            if(!rep && aux_fitness < best_fitness) {
                best_solution = aux;
                best_fitness = aux_fitness;
            }

            // add individual to best vector
            best[ELITISM - 1 - i] = best_solution;
        }
    }

    // assign best individuals to first positions of the population
    for(i = 0; i < ELITISM; i++) {
        memmove(initial_population.genotypes[i], best[i], (n_cities - 1) * sizeof(unsigned int));
    }

    // sort children by ascending fitness (prime_population)
    SOLUTION *ptr = &(prime_population.genotypes[ELITISM]);
    population_tim_sort(ptr, POPULATION_SIZE - ELITISM);
    for(i = ELITISM; i < POPULATION_SIZE; i++) {
        memcpy(initial_population.genotypes[i], prime_population.genotypes[i], (n_cities - 1) * sizeof(unsigned int));
    }
}

unsigned int genetic_algo() {
    unsigned int solution_fitness = 0;
    int i = 0;

    printf("INITIAL POPULATION\n");
    srand48((unsigned)time(NULL));
    int success = generate_initial_population();
    if(!success) {
        printf("ERROR GENERATING INITIAL POPULATION!\n");
    }
    else {
        // prime_population initialization (temporary storage of children genotypes)
        for(i = ELITISM; i < POPULATION_SIZE; i++) {
            prime_population.genotypes[i] = (SOLUTION)calloc(n_cities - 1, sizeof(unsigned int));
            if(prime_population.genotypes[i] == NULL) {
                break;
            }
        }
        if(i < POPULATION_SIZE) {
            printf("ERROR INITIALIZING PRIME POPULATION!\n");
        }
        else {
            print_population(initial_population);
            printf("\n");

            solution_fitness = fitness(solution);
            int fathers[POPULATION_SIZE - ELITISM];
            SOLUTION best_elite = NULL, best_descendant= NULL, best_pop = NULL;
            unsigned int generated_populations = 1, best_sol_pop = generated_populations - 1;
            while(generated_populations <= GENERATION_LIMIT) {
                printf("ITERATION: %u, SELECTION\n", generated_populations);
                for(i = 0; i < POPULATION_SIZE - ELITISM; i++) {
                    // selection by tournament k=2
                    fathers[i] = select_father(i);
                }
                printf("\n");

                printf("ITERATION: %u, CROSSING\n", generated_populations);
                for(i = 0; i < POPULATION_SIZE - ELITISM; i += 2) {
                    crossing(fathers[i], fathers[i + 1], i);
                }

                printf("ITERATION: %u, MUTATION\n", generated_populations);
                for(i = ELITISM; i < POPULATION_SIZE; i++) {
                    mutation(i);
                }
                printf("\n");

                printf("ITERATION: %u, REPLACEMENT\n", generated_populations);
                replacement();
                print_population(initial_population);
                printf("\n");

                // global optimum
                best_elite = initial_population.genotypes[ELITISM - 1];
                best_descendant = initial_population.genotypes[ELITISM];
                best_pop = (fitness(best_elite) <= fitness(best_descendant))?
                    best_elite : best_descendant;
                if(fitness(best_pop) < solution_fitness) {
                    memcpy(solution, best_pop, (n_cities - 1) * sizeof(unsigned int));
                    solution_fitness = fitness(best_pop);
                    best_sol_pop = generated_populations;
                }

                generated_populations++;
            }

            printf("\nBEST SOLUTION: \n");
            printf("WALK: ");
            for(i = 0; i < n_cities - 1; i++) {
                printf("%u ", solution[i]);
            }
            printf("\nFITNESS (km): %u\n", solution_fitness);
            printf("ITERATION: %u\n", best_sol_pop);
        }
    }
    free_population(&prime_population);
    free_population(&initial_population);

    return solution_fitness;
}


/******GENETIC ALGORITHM FROM FILE******/
FILE *random_file = NULL;

void generate_random_solution_file(SOLUTION genotype) {
    int i = 0, j = 0;
    double r = 0.0;
    for(; i < n_cities - 1; i++) {
        fscanf(random_file, "%lf\r\n", &r);
        genotype[i] = 1 + floor(r * (n_cities - 1));

        for(j = 0; j < i; j++) {
            if(genotype[i] == genotype[j]) {
                int repeated = 0;
                do {
                    repeated = 0;
                    genotype[i] = genotype[i] % (n_cities - 1) + 1;
                    for(j = 0; j < i; j++) {
                        if(genotype[i] == genotype[j]) {
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

void generate_greedy_solution_file(SOLUTION genotype) {
    int i = 0, j = 0, k = 0;
    double r = 0.0;
    int repeated = 0;
    unsigned int departure_city = 0, min_dist_city = 0, min_dist = 0;

    // first gene with random allele; following are calculated according to minimum distance
    fscanf(random_file, "%lf\r\n", &r);
    genotype[0] = 1 + floor(r * (n_cities - 1));
    for(i = 1; i < n_cities - 1; i++) {
        departure_city = genotype[i - 1];

        // alleles already assigned are not taken into account (cities already in the walk)
        min_dist_city = departure_city;
        do {
            min_dist_city = min_dist_city % (n_cities - 1) + 1;
            repeated = 0;
            for(k = 0; k < i; k++) {
                if(min_dist_city == genotype[k]) {
                    repeated = 1;
                    break;
                }
            }
        } while(repeated);
        min_dist = (departure_city > min_dist_city)? d[departure_city - 1][min_dist_city]
            : d[min_dist_city - 1][departure_city];
        genotype[i] = min_dist_city;

        j = 1;
        while(j < (n_cities - i - 1)) {
            // alleles already assigned are not taken into account (cities already in the walk)
            do {
                repeated = 0;
                min_dist_city = min_dist_city % (n_cities - 1) + 1;
                for(k = 0; k < i; k++) {
                    if(min_dist_city == genotype[k]) {
                        repeated = 1;
                        break;
                    }
                }
            } while(repeated);

            if(departure_city > min_dist_city && d[departure_city - 1][min_dist_city] < min_dist) {
                genotype[i] = min_dist_city;
                min_dist = d[departure_city - 1][min_dist_city];
            }
            else if(departure_city < min_dist_city && d[min_dist_city - 1][departure_city] < min_dist) {
                genotype[i] = min_dist_city;
                min_dist = d[min_dist_city - 1][departure_city];
            }

            j++;
        }
    }
}

int generate_initial_population_file() {
    int success = 0;

    unsigned int solution_fitness = -1;
    int i = 0;
    // random genotypes
    initial_population.genotypes[0] = (SOLUTION)calloc((n_cities - 1), sizeof(unsigned int));
    if(initial_population.genotypes[0] == NULL) {
        return success;
    }
    generate_random_solution_file(initial_population.genotypes[0]);
    solution_fitness = fitness(initial_population.genotypes[0]);
    for(i = 1; i < POPULATION_SIZE/2; i++) {
        initial_population.genotypes[i] = (SOLUTION)calloc((n_cities - 1), sizeof(unsigned int));
        if(initial_population.genotypes[i] == NULL) {
            break;
        }
        generate_random_solution_file(initial_population.genotypes[i]);

        if(fitness(initial_population.genotypes[i]) < solution_fitness) { // best solution of the population
            memcpy(solution, initial_population.genotypes[i], (n_cities - 1) * sizeof(unsigned int));
            solution_fitness = fitness(solution);
        }
    }
    // semi-random genotypes
    for(; i < POPULATION_SIZE; i++) {
        initial_population.genotypes[i] = (SOLUTION)calloc((n_cities - 1), sizeof(unsigned int));
        if(initial_population.genotypes[i] == NULL) {
            break;
        }
        generate_greedy_solution_file(initial_population.genotypes[i]);

        if(fitness(initial_population.genotypes[i]) < solution_fitness) { // best solution of the population
            memcpy(solution, initial_population.genotypes[i], (n_cities - 1) * sizeof(unsigned int));
            solution_fitness = fitness(solution);
        }
    }

    if(i == POPULATION_SIZE) {
        success = 1;
    }

    return success;
}

int select_father_file(unsigned int tournament) {
    int i = 0, j = 0;
    double r = 0.0;
    int father_index = 0;

    /* tournament of k=2 random genotypes (should be programmed with vectors and loops parametrized
     * with k for the algorithm to be generic
    */
    fscanf(random_file, "%lf\r\n", &r);
    i = floor(r * POPULATION_SIZE);
    fscanf(random_file, "%lf\r\n", &r);
    j = floor(r * POPULATION_SIZE);
    if(fitness(initial_population.genotypes[i]) <= fitness(initial_population.genotypes[j])) {
        father_index = i;
    }
    else {
        father_index = j;
    }
    printf("\tTOURNAMENT %u: %d %d %d wins\n", tournament, i, j, father_index);

    return father_index;
}

void crossing_file(int index_1, int index_2, int ite) {
    SOLUTION p1 = initial_population.genotypes[index_1];
    SOLUTION p2 = initial_population.genotypes[index_2];
    SOLUTION h1 = prime_population.genotypes[ite + ELITISM];
    SOLUTION h2 = prime_population.genotypes[ite + ELITISM + 1];
    double r = 0.0;
    int pt_1 = 0, pt_2 = 0, tmp = 0;
    int n_cut_area = 0;
    int i = 0, j = 0, k1 = 0, k2 = 0, z = 0;
    int rep = 0;

    fscanf(random_file, "%lf\r\n", &r);
    printf("\tCROSSING: (%d, %d) (RANDOM: %lf)\n", ite, ite + 1, r);
    printf("\t\tFATHER: = {FITNESS (km): %u, WALK: ", fitness(p1));
    for(i = 0; i < n_cities - 1; i++) {
        printf("%u ", p1[i]);
    }
    printf("}\n");
    printf("\t\tFATHER: = {FITNESS (km): %u, WALK: ", fitness(p2));
    for(i = 0; i < n_cities - 1; i++) {
        printf("%u ", p2[i]);
    }
    printf("}\n");

    if(r <= CROSS_PROB) {
        fscanf(random_file, "%lf\r\n", &r);
        pt_1 = floor(r * (n_cities - 1));
        fscanf(random_file, "%lf\r\n", &r);
        pt_2 = floor(r * (n_cities - 1));
        printf("\t\tCUTS: (%d, %d)\n", pt_1, pt_2);
        if(pt_1 > pt_2) {
            tmp = pt_1;
            pt_1 = pt_2;
            pt_2 = tmp;
        }
        n_cut_area = pt_2 - pt_1 + 1;

        memcpy(h1 + pt_1, p1 + pt_1, n_cut_area * sizeof(unsigned int));
        memcpy(h2 + pt_1, p2 + pt_1, n_cut_area * sizeof(unsigned int));
        i = 0;
        k1 = (pt_2 + 1) % (n_cities - 1), k2 = k1, z = k1;
        while(i < (n_cities - 1) - n_cut_area) {
            // p2-h1 inheritance
            do {
                rep = 0;
                for(j = pt_1; j <= pt_2; j++) {
                    if(p2[k1] == h1[j]) {
                        k1= (k1 + 1) % (n_cities - 1);
                        rep = 1;
                        break;
                    }
                }
            } while(rep);
            h1[z] = p2[k1];
            // p1-h2 inheritance
            do {
                rep = 0;
                for(j = pt_1; j <= pt_2; j++) {
                    if(p1[k2] == h2[j]) {
                        k2 = (k2 + 1) % (n_cities - 1);
                        rep = 1;
                        break;
                    }
                }
            } while(rep);
            h2[z] = p1[k2];

            z = (z + 1) % (n_cities - 1);
            k1 = (k1 + 1) % (n_cities - 1);
            k2 = (k2 + 1) % (n_cities - 1);
            i++;
        }

        printf("\t\tCHILD: = {FITNESS (km): %u, WALK: ", fitness(h1));
        for(i = 0; i < n_cities - 1; i++) {
            printf("%u ", h1[i]);
        }
        printf("}\n");
        printf("\t\tCHILD: = {FITNESS (km): %u, WALK: ", fitness(h2));
        for(i = 0; i < n_cities - 1; i++) {
            printf("%u ", h2[i]);
        }
        printf("}\n");
    }
    else { // children are identical to their fathers
        printf("\t\tNOT CROSSED\n");
        memcpy(h1, p1, (n_cities - 1) * sizeof(unsigned int));
        memcpy(h2, p2, (n_cities - 1) * sizeof(unsigned int));
    }
    printf("\n");
}

void mutation_file(int index_child) {
    SOLUTION child = prime_population.genotypes[index_child];
    double r = 0.0, random = 0.0;
    int pos_i = 0, pos_j = 0;
    int i = 0;

    printf("\tINDIVIDUAL %d\n", index_child - ELITISM);
    printf("\tPREVIOUS WALK: ");
    for(; i < n_cities - 1; i++) {
        printf("%u ", child[i]);
    }
    printf("\n");

    for(i = 0; i < n_cities - 1; i++) {
        fscanf(random_file, "%lf\r\n", &random);
        if(random <= MUTA_PROB) {
            pos_i = i;
            fscanf(random_file, "%lf\r\n", &r);
            pos_j = floor(r * (n_cities - 1));
            printf("\t\tPOSITION: %d (RANDOM %lf) INTERCHANGED WITH: %d\n", i, random, pos_j);
            permutation(&child, pos_i, pos_j);
        }
        else {
            // not mutated
            printf("\t\tPOSITION: %d (RANDOM %lf) NOT MUTATED\n", i, random);
        }
    }

    printf("\tNEW WALK: ");
    for(i = 0; i < n_cities - 1; i++) {
        printf("%u ", child[i]);
    }
    printf("\n");
    printf("\n");
}

unsigned int genetic_algo_file(char* trace) {
    unsigned int solution_fitness = 0;

    random_file = fopen(trace, "r");
    if(random_file != NULL) {
        printf("INITIAL POPULATION\n");
        srand48((unsigned)time(NULL));
        int success = generate_initial_population_file();
        if(!success) {
            printf("ERROR GENERATING INITIAL POPULATION!\n");
        }
        else {
            int i = 0;

            // prime_population initialization (temporary storage of children genotypes)
            for(i = ELITISM; i < POPULATION_SIZE; i++) {
                prime_population.genotypes[i] = (SOLUTION)calloc(n_cities - 1, sizeof(unsigned int));
                if(prime_population.genotypes[i] == NULL) {
                    break;
                }
            }
            if(i < POPULATION_SIZE) {
                printf("ERROR INITIALIZING PRIME POPULATION!\n");
            }
            else {
                print_population(initial_population);
                printf("\n");

                solution_fitness = fitness(solution);
                int fathers[POPULATION_SIZE - ELITISM];
                SOLUTION best_elite = NULL, best_descendant= NULL, best_pop = NULL;
                unsigned int generated_populations = 1, best_sol_pop = generated_populations - 1;
                while(generated_populations <= GENERATION_LIMIT) {
                    printf("ITERATION: %u, SELECTION\n", generated_populations);
                    for(i = 0; i < POPULATION_SIZE - ELITISM; i++) {
                        // selection by tournament k=2
                        fathers[i] = select_father_file(i);
                    }
                    printf("\n");

                    printf("ITERATION: %u, CROSSING \n", generated_populations);
                    for(i = 0; i < POPULATION_SIZE - ELITISM; i += 2) {
                        crossing_file(fathers[i], fathers[i + 1], i);
                    }

                    printf("ITERATION: %u, MUTATION\n", generated_populations);
                    for(i = ELITISM; i < POPULATION_SIZE; i++) {
                        mutation_file(i);
                    }
                    printf("\n");

                    printf("ITERATION: %u, REPLACEMENT\n", generated_populations);
                    replacement();
                    print_population(initial_population);
                    printf("\n");

                    // global optimum
                    best_elite = initial_population.genotypes[ELITISM - 1];
                    best_descendant = initial_population.genotypes[ELITISM];
                    best_pop = (fitness(best_elite) <= fitness(best_descendant))?
                        best_elite : best_descendant;
                    if(fitness(best_pop) < solution_fitness) {
                        memcpy(solution, best_pop, (n_cities - 1) * sizeof(unsigned int));
                        solution_fitness = fitness(best_pop);
                        best_sol_pop = generated_populations;
                    }

                    generated_populations++;
                }

                printf("\nBEST SOLUTION: \n");
                printf("WALK: ");
                for(i = 0; i < n_cities - 1; i++) {
                    printf("%u ", solution[i]);
                }
                printf("\nFITNESS (km): %u\n", solution_fitness);
                printf("ITERATION: %u\n", best_sol_pop);
            }
        }
        free_population(&prime_population);
        free_population(&initial_population);

        fclose(random_file);
    }
    else {
        printf("ERROR OPENING RANDOM FILE %s.\n", trace);
    }

    return solution_fitness;
}
