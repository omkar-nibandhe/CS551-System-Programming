all: module server client

module: non-space.c alnum-quote.c
	gcc -g -Wall -std=c11 -fPIC -shared alnum-quote.c -o alnum-quote.mod
	gcc -g -Wall -std=c11 -fPIC -shared non-space.c -o non-space.mod

server: main.c previous_main.c
	gcc -L. -g -o word-countd main.c -ldl
	LD_LIBRARY_PATH=. ./word-countd .

client: client.c client.h
	gcc -L. -g -o word-count client.c


clean:
	rm -rf word-countd
	rm -rf word-count
	rm -rf SERVER_FIFO
	rm -rf alnum-quote.mod
	rm -rf non-space.mod
