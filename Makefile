

all:
	gcc -std=c11 -Wall -Ofast aetg.c -o aetg -fopenmp -march=native -mtune=native -g
