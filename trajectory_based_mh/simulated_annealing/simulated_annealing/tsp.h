typedef unsigned int** DISTANCES; // city distances matrix (lower triangular)
typedef unsigned int* SOLUTION; // vector of length n-1 representing a walk

extern unsigned int n_cities;
extern DISTANCES d;
extern SOLUTION solution;

int initialize_data(unsigned int n, char* data_file); // loads distance matrix and n_cities
void free_data(); // frees memory
unsigned int fitness(SOLUTION s); // calculates the walk distance, departure city included
