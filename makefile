testServer: build
	pytest

runServer: build
	./chat 2115

stopServer:
	kill `pidof chat`

build: client.c server.c chat.c
	gcc -pthread client.c server.c chat.c -O0 -Wall -o chat -I.
