# Metaheuristics
A collection of metaheuristic algorithms for the **Traveller Salesman Problem (TSP)**. They are all implemented in C, and divided in two categories:
* **Trajectory-based**:
    1. Local Search
    2. Tabu Search
    3. Simulated Annealing
* **Population-based**:
    * Genetic Algorithm
    
## Compilation
To compile each metaheuristic program, run the `make` command in the root directory of its corresponding project (it compiles from the Makefile).

    ./make


## Execution
To execute a metaheuristic program, run the command:
        
    ./program_name -a [n] [distances_file]
        
Where `[n]` is the number of cities and `[distances_file]` is the path to the TSP distance matrix file.

If you want to test the metaheuristic with a specific trace (search from file), run the command with the option `-f` and an additional argument `[random_file]`, which is the path to the random number sequence file for the trace:

    ./program_name -f [random_file] [n] [distances_file]
    
In both cases, beware that **the number of cities in the distances file and that in the program (`[n]`) have to be the same**, in order to the program to work well.

Results are printed to stdout.