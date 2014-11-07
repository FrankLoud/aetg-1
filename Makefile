

all:
	gcc -std=c11 -Ofast aetg.c -o aetg -fopenmp -march=native -mtune=native -g
