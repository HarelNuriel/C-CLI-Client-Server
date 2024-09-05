#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 1024

/*
Takes in a command and the a pointer to an array of strings and puts each lines
of the command's output in its respective index
*/
void execute_command(int connected_sock, char command[], char *output) {

  FILE *fp;
  char val[1024];
  int i = 0, j = 0, flag;

  fp = popen(command, "r");
  if (fp == NULL) {
    return;
  }

  /* Read the output a line at a time - output it. */
  while (fgets(val, sizeof(output), fp) != NULL) {

    /*
     Iterate through the command output and fill the output buffer.
     Leave one empty space in the output buffer for null terminator to avoid seg
     fault
     */
    while (val[i] != '\0' && j < BUF_SIZE - 2) {
      output[j] = val[i];
      i++;
      j++;
    }

    // If the buffer is full send it to the client and continue iterating
    // through the command output
    if (j == BUF_SIZE - 2) {

      // Add null terminator to end the string and avoid seg fault
      output[BUF_SIZE - 1] = '\0';
      flag = write(connected_sock, output, BUF_SIZE);

      if (flag < 0) {
        perror("Error sending the output buffer");
        continue;
      }

      memset(output, 0, BUF_SIZE);
      j = 0;
      continue;
    }

    i = 0;
  }

  output[j + 1] = '\0';
  flag = write(connected_sock, output, j + 2);

  if (flag < 0) {
    perror("Error sending the output buffer");
  }

  // close
  pclose(fp);

  // Execution complete
  return;
}

int main(int argc, char **argv) {

  if (argc < 3) {
    printf("Usage: server <bind_ip> <bind_port>");
    exit(1);
  }

  const char *IP = argv[1];
  int PORT = strtol(argv[2], NULL, 10);

  if (PORT > 65535 || PORT < 0) {
    printf("Error: Invalid port number %d", PORT);
    exit(1);
  }

  int connection_status, sock, connected_sock, flag, i;
  ssize_t buf_len;
  int opt = 1;
  char buffer[BUF_SIZE], command[256], *output = (char *)malloc(BUF_SIZE);
  struct sockaddr_in server;
  socklen_t sock_len = sizeof(server);

  // Creating the socket
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    printf("Error Creating Socket.");
    return -1;
  }

  // Setting socket options to reuse the IP address and port
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt)) < 0) {
    printf("Error Setting socket options.");
    return -1;
  }
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  flag = inet_pton(AF_INET, IP, (struct sockaddr_in *)&server.sin_addr);

  // Attaching (Binding) the socket to port 8080
  if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
    printf("Error binding the socket to port %d.", PORT);
    return -1;
  }

  // Listening for new connections
  if (listen(sock, 3)) {
    printf("Error Listening for connections.");
    return -1;
  }

  // Accepting the incoming connection
  connected_sock = accept(sock, (struct sockaddr *)&server, &sock_len);
  if (connected_sock < 0) {
    printf("Error accepting the socket.");
    return -1;
  }

  printf("start\n");
  while (1) {
    i = 0;
    memset(buffer, 0, BUF_SIZE);
    memset(command, 0, 256);
    memset(output, 0, BUF_SIZE);
    buf_len = read(connected_sock, buffer, BUF_SIZE);

    if (buf_len < 0) {
      perror("ERROR in reading from socket");
      exit(1);
    }

    // Escape this loop, if the client sends message "exit"
    if (!bcmp(buffer, "exit", 4)) {
      break;
    }

    strcpy(command, buffer);

    execute_command(connected_sock, command, output);

    if (buf_len < 0) {
      perror("ERROR in writing to socket");
    }
  }

  free(output);

  return 0;
}
