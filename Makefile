CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb

lit: lit.c
	$(CC) $(CFLAGS) -o lit lit.c
