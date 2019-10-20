#include <stdio.h>
#include <stdlib.h>
#include "tsp.h"

unsigned int n_cities = 0;
DISTANCES d = NULL;
SOLUTION solution = NULL;

int initialize_data(unsigned int n, char* data_file) {
    int success = 0;

    if(n > 1 && data_file != NULL) {
        n_cities = n;

        FILE *fp = fopen(data_file, "r");
        if(fp != NULL) {
            d = (unsigned int**)calloc(n_cities - 1, sizeof(unsigned int*));
            if(d != NULL) {
                int i = 0, j = 0;
                int read = 1;
                for(; i < n_cities - 1 && read; i++) {
                    d[i] = (unsigned int*)calloc(i + 1, sizeof(unsigned int)); // no need for allocating n positions in every row
                    if(d[i] == NULL) {
                        free_data();
                        break;
                    }
                    read = fscanf(fp, "%u", &d[i][0]);
                    if(read < 1) {
                        free_data();
                        break;
                    }
                    for(j = 1; j < i + 1; j++) {
                        read = fscanf(fp, "%u", &d[i][j]);
                        if(read < 1) {
                            free_data();
                            break;
                        }
                    }
                }

                if(d != NULL) {
                    solution = (unsigned int*)calloc(n_cities - 1, sizeof(unsigned int));
                    success = 1;
                }
            }

            fclose(fp);
        }
    }

    return success;
}

void free_data() {

    if(d != NULL) {
        int i = 0;
        for(; i < n_cities - 1; i++) {
            if(d[i] != NULL) {
                free(d[i]);
                d[i] = NULL;
            }
        }
        free(d);
        d = NULL;
    }
    if(solution != NULL) {
        free(solution);
        solution = NULL;
    }
    n_cities = 0;
}

unsigned int fitness(SOLUTION solution) {
    unsigned int distance = 0;

    if(solution != NULL) {
        unsigned int departure_city = 0;
        unsigned int arrival_city = solution[0];
        distance = d[arrival_city - 1][0]; // departure city -> first city of the walk

        int i = 1;
        for(; i < n_cities - 1; i++) {
            departure_city = solution[i - 1];
            arrival_city = solution[i];
            distance += (arrival_city >= departure_city) ? d[arrival_city - 1][departure_city] : d[departure_city - 1][arrival_city];
        }
        departure_city = solution[n_cities - 2];
        arrival_city = 0;
        distance += d[departure_city - 1][arrival_city]; // last city of the walk -> departure city
    }

    return distance;
}
