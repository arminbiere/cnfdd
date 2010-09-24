CC=gcc
CFLAGS=-Wall -g
cnfdd: cnfdd.c makefile
	$(CC) $(CFLAGS) -o $@ cnfdd.c
clean:
	rm -f cnfdd
