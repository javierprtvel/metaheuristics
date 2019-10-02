GCC = gcc -Wall

MAIN = a.out

DEPS = problema_viajante.h mh_ce.h sort.h

INCLUDES = -I $(DEPS)
SOURCES = main.c problema_viajante.c mh_ce.c

OBJECTS = $(SOURCES: .c = .o)

$(MAIN): $(OBJECTS)
	$(GCC) -o $(MAIN) $(OBJECTS) -lm


%.o: %.c $(DEPS)
	$(GCC) -c $< $(INCLUDES)


cleanall: clean
	rm -f $(MAIN)

clean:
	rm -f *.o *~
