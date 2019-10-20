#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mh_genetic_algo.h"
#include "tsp.h"

int main(int argc, char** argv) {
    unsigned int result;
    int start;

    if(argc >= 5 && strncmp(argv[1], "-f", strlen("-f")) == 0) {
        start = initialize_data(atoi(argv[3]), argv[4]);
        if(!start) {
            printf("Error reading from file: could not load data correctly.\n");
            exit(1);
        }
        result = genetic_algo_file(argv[2]);
    }
    else if(argc == 4 && strncmp(argv[1], "-a", strlen("-a")) == 0) {
        start = initialize_data(atoi(argv[2]), argv[3]);
        if(!start) {
            printf("Error initializing data: could not load data correctly.\n");
            exit(1);
        }
        result = genetic_algo();
    }
    else {
        printf("Error: incorrect parameters.\n");
        printf("Command for genetic algorithm: %s -a [n_cities] [distances_file]\n", argv[0]);
        printf("Command for genetic algorithm from file: %s -f [random_file] [n_cities] [distances_file]\n", argv[0]);
        exit(1);
    }

    if(!result) {
        printf("Error executing the metaheuristic. Could not calculate the solution.\n\n");
    }

    free_data();

    exit(0);
}
