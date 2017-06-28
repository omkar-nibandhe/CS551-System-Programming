all: server client

server: main.c main.h semaphore.c previous_main.c word-store.c
	gcc -g -o word-countd main.c -lrt
	./word-countd .

client: client.c client.h
	gcc -g -o word-count client.c -lrt


clean:
	rm -rf word-countd
	rm -rf word-count
