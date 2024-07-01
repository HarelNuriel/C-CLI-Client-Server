#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <asm-generic/socket.h>

#define PORT 8080
#define IP "127.0.0.1"
#define BUF_SIZE 1024

void execute_command(char command[], char* output){

    FILE *fp;
    char val [1024];
    int i = 0, j = 0;

    fp = popen(command, "r");
    if (fp == NULL) {
        return;
        exit(1);
    }

    /* Read the output a line at a time - output it. */
    while (fgets(val, sizeof(output), fp) != NULL) {
        //printf("%s", val);
        if ((val[i] >= 'a' && val[i] <= 'z') || (val[i] >= 'A' && val[i] <= 'Z'))
            while (val[i] != '\0') {
                output[j] = val[i];
                i++;
                j++;
            }

        i = 0;
    }

    output[j + 1] = '\0';

    /* close */
    pclose(fp);

    return;
}


int main() {

    int connection_status, sock, connected_sock, flag, i;
    ssize_t buf_len;
    int opt = 1;
    char buffer [BUF_SIZE], command [256], *output = (char*)malloc(BUF_SIZE);
    struct sockaddr_in server;
    socklen_t sock_len = sizeof(server);

    // Creating the socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Error Creating Socket.");
        return -1;
    }

    // Setting socket options to reuse the IP address and port
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        printf("Error Setting socket options.");
        return -1;
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    flag = inet_pton(AF_INET, "127.0.0.1", (struct sockaddr_in *)&server.sin_addr);

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
        memset(command, 0, BUF_SIZE);
        memset(output, 0, BUF_SIZE);
        buf_len = read(connected_sock, buffer, BUF_SIZE);

        if (buf_len < 0){
            perror("ERROR in reading from socket");
            exit(1);
        }

        strcpy(command, buffer);

        execute_command(command, output);
        buf_len = write(connected_sock, output, BUF_SIZE);

        if (buf_len < 0){
            perror("ERROR in writing to socket");
            exit(1);
        }

        //free(command);
        // escape this loop, if the client sends message "quit"
        if (!bcmp(buffer, "exit", 4))
            break;
    }

    free(output);

    return 0;
}