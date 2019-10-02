GCC = gcc -Wall

MAIN = a.out

DEPS = problema_viajante.h mh_busq_sa.h

INCLUDES = -I $(DEPS)
SOURCES = main.c problema_viajante.c mh_busq_sa.c

OBJECTS = $(SOURCES: .c = .o)

$(MAIN): $(OBJECTS)
	$(GCC) -o $(MAIN) $(OBJECTS) -lm


%.o: %.c $(DEPS)
	$(GCC) -c $< $(INCLUDES)


cleanall: clean
	rm -f $(MAIN)

clean:
	rm -f *.o *~
