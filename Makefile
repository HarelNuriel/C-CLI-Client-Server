CC = gcc


make:
	$(CC) client.c -o client
	$(CC) server.c -o server
