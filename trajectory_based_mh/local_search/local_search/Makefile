GCC = gcc -Wall

MAIN = a.out

DEPS = tsp.h mh_local_search.h

INCLUDES = -I $(DEPS)
SOURCES = main.c tsp.c mh_local_search.c

OBJECTS = $(SOURCES: .c = .o)

$(MAIN): $(OBJECTS)
	$(GCC) -o $(MAIN) $(OBJECTS) -lm


%.o: %.c $(DEPS)
	$(GCC) -c $< $(INCLUDES)


cleanall: clean
	rm -f $(MAIN)

clean:
	rm -f *.o *~
