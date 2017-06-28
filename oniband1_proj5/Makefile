all: server client

server: server.c
	gcc -g -pthread -o word-countd server.c -lrt

client: client.c
	gcc -g -pthread -o word-count client.c -lrt


clean:
	rm -rf word-countd
	rm -rf word-count
