GCC = gcc -Wall

MAIN = viajante_busq_local_primer_mejor.out

DEPS = problema_viajante.h mh_busq_local_primer_mejor.h

INCLUDES = -I $(DEPS)
SOURCES = main.c problema_viajante.c mh_busq_local_primer_mejor.c

OBJECTS = $(SOURCES: .c = .o)

$(MAIN): $(OBJECTS)
	$(GCC) -o $(MAIN) $(OBJECTS) -lm


%.o: %.c $(DEPS)
	$(GCC) -c $< $(INCLUDES)


cleanall: clean
	rm -f $(MAIN)

clean:
	rm -f *.o *~
