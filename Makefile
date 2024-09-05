CC = gcc


make:
	$(CC) client.c -o client
	$(CC) server.c -o server

debug:
	$(CC) client.c -g -o client
	$(CC) server.c -g -o server