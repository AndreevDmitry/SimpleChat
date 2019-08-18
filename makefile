testServer: build
	pytest

runServer: build
	./chat 2115 & echo $$! > /tmp/chat.pid

stopServer:
	kill `cat /tmp/chat.pid` && rm "/tmp/chat.pid"

build: client.c server.c chat.c
	gcc -pthread client.c server.c chat.c -O0 -Wall -o chat -I.
