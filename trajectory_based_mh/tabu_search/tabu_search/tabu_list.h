typedef struct {
    unsigned int new_pos_i;
    unsigned int new_pos_j;
} PERMUTATION;

typedef struct {
    unsigned int city_index;
    unsigned int insertion_index;
} INSERTION;

typedef void * tabu_list;

void create_tabu_list(tabu_list *lt, unsigned int capacity);
void free_tabu_list(tabu_list *lt);
//void insert_tabu(tabu_list lt, PERMUTATION in);
//int is_tabu(tabu_list lt, PERMUTATION in); // returns 0 if not found
void insert_tabu(tabu_list lt, INSERTION in);
int is_tabu(tabu_list lt, INSERTION in); // returns 0 if not found
void empty_tabu_list(tabu_list lt);
void print_tabu_list(tabu_list lt);
