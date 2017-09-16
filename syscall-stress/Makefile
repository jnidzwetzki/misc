CC=gcc
CFLAGS=-std=c99 -g 
OBJ = experiment.o

main: $(OBJ)
	$(CC) -o main $(OBJ) -lpthread

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	rm -f *.o
	rm -f main
