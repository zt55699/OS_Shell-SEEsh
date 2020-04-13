CC=gcc
CFLAGS=-Wall -Werror -g

SEEsh: SEEsh.o
	$(CC) $(CFLAGS) -o $@ $^

SEEsh.o: SEEsh.c 
	$(CC) $(CFLAGS) -c $<

.PHONY: run, gdb, valgrind, clean

run: SEEsh
	./SEEsh

clean:
	rm *.o SEEsh

gdb: SEEsh
	gdb ./SEEsh

valgrind: SEEsh
	valgrind ./SEEsh --leak-check=full

