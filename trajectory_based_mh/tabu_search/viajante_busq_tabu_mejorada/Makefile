GCC = gcc -Wall

MAIN = a.out

DEPS = problema_viajante.h mh_busq_tabu.h lista_tabu.h

INCLUDES = -I $(DEPS)
SOURCES = main.c problema_viajante.c mh_busq_tabu.c lista_tabu.c

OBJECTS = $(SOURCES: .c = .o)

$(MAIN): $(OBJECTS)
	$(GCC) -o $(MAIN) $(OBJECTS) -lm


%.o: %.c $(DEPS)
	$(GCC) -c $< $(INCLUDES)


cleanall: clean
	rm -f $(MAIN)

clean:
	rm -f *.o *~
