build:
		gcc -O2 -c main.c
		gcc -o main.out main.o -lusb-1.0

.PHONY: build
