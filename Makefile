

all:
	gcc -std=c11 -Og aetg.c -o aetg -fopenmp -march=native -mtune=native -g
