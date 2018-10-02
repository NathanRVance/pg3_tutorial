all: serve cli

serve:
	gcc server.c sockets.h -o server -lpthread

cli:
	gcc client.c sockets.h -o client
