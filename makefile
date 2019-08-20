testServer: build
	pytest

runServer: build
	./chat 2115 & echo $$! > /tmp/chatServer.pid

runClient: build
	./chat 127.0.0.1 2115 

stopServer:
	kill `cat /tmp/chatServer.pid` && rm "/tmp/chatServer.pid"

build: client.c server.c chat.c
	gcc -pthread client.c server.c chat.c -O0 -Wall -o chat -I.
