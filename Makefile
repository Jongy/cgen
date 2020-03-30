all: libcgen.a

libcgen.a: cgen.c gencall.S
	gcc -c cgen.c gencall.S
	ar r $@ cgen.o gencall.o

main: libcgen.a main.c
	gcc -o main main.c libcgen.a

run: main
	./main

clean:
	rm -f *.o libcgen.a main
