all: mem

mem: mem.o
	gcc -g mem.o -o mem

mem.o: mem.c
	gcc -g -c mem.c

clean:
	rm -f *.o
	rm -f mem
	rm -f swap*
