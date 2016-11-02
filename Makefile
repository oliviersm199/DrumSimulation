CC= mpicc
CFLAGS= -std=c99

all: grid_4_4.c grid_512_512.c
	$(CC) $(CFLAGS) grid_4_4.c -o grid_4_4
	$(CC) $(CFLAGS) grid_512_512.c -o grid_512_512 
