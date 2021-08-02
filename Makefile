CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb

lit: src/main.c
	$(CC) $(CFLAGS) -o lit src/main.c
