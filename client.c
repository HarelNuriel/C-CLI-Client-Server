#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 1024

int main(int argc, char **argv) {

  if (argc < 3) {
    printf("Usage: client <server_ip> <server_port>");
    exit(1);
  }

  const char *IP = argv[1];
  int PORT = strtol(argv[2], NULL, 10);

  if (PORT > 65535 || PORT < 0) {
    printf("Error: Invalid port number %d", PORT);
    exit(1);
  }

  int connection_status, sock;
  ssize_t buf_len;
  char buffer[BUF_SIZE], command[BUF_SIZE];
  struct sockaddr_in serv_addr;

  sock = socket(AF_INET, SOCK_STREAM, 0);

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);
  inet_pton(AF_INET, IP, (struct sockaddr_in *)&serv_addr.sin_addr);

  connection_status =
      connect(sock, (struct sockaddr_in *)&serv_addr, sizeof(serv_addr));

  if (connection_status < 0) {
    printf("Connecction Failed\n");
    return -1;
  }

  printf("Connected\n");

  while (1) {
    printf("kali@server:~/$ ");
    memset(buffer, 0, BUF_SIZE);
    fgets(buffer, BUF_SIZE, stdin);

    buf_len = write(sock, buffer, strlen(buffer));

    if (buf_len < 0) {
      perror("ERROR while writing to socket");
      exit(1);
    }

    // escape this loop, if the server sends message "exit"
    if (!bcmp(buffer, "exit", 4))
      break;

    buf_len = read(sock, buffer, BUF_SIZE);

    if (buf_len < 0) {
      perror("Error while reading from socket");
      continue;
    }

    while (buf_len == BUF_SIZE) {
      printf("%s \n", buffer);

      buf_len = read(sock, buffer, BUF_SIZE);

      if (buf_len < 0) {
        perror("Error while reading from socket");
        continue;
      }
    }

    printf("%s \n", buffer);
  }

  return 0;
}
