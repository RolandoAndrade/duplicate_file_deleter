all:
	gcc -c dup.c -o dup.o
	gcc dup.o -o dup

clean:
	rm dup.o dup