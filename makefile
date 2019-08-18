testServer: server
	pytest

runServer: server
	./server & echo $$! > /tmp/server.pid

stopServer:
	kill `cat /tmp/server.pid` && rm "/tmp/server.pid"

server: server.c
	gcc server.c -O0 -Wall -o server

client: client.c
	gcc client.c -O0 -Wall -pthread -o client

all: server client
