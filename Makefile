all: run

cgen: cgen.c gencall.S
	gcc -o cgen cgen.c gencall.S

run: cgen
	./cgen
