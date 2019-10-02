typedef unsigned int** DISTANCIAS; //matriz de distancias entre ciudades (triangular inferior)
typedef unsigned int* SOLUCION; //vector de n-1 elementos que representa una permutacion o recorrido

extern unsigned int n_ciudades;
extern DISTANCIAS d;
extern SOLUCION solucion;

int inicializa_datos(unsigned int n, char* archivo_datos); //carga los datos de las distancias (y el nº de ciudades) leyendo el archivo especificado
void elimina_datos(); //libera la memoria dinámica reservada
unsigned int objetivo(SOLUCION s); //calcula la distancia del recorrido, teniendo en cuenta la ciudad de partida
