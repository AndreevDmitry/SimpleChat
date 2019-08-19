testServer: build
	pytest

runServer: build
	./chat 2115 & echo $$! > /tmp/chatServer.pid

stopServer:
	kill `cat /tmp/chatServer.pid` && rm "/tmp/chatServer.pid"

build: client.c server.c chat.c
	gcc -pthread client.c server.c chat.c -O0 -Wall -o chat -I.
