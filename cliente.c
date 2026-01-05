#include <sys/socket.h>
#include <netinet/in.h> 
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>


void ClienteHTTP(char *dirIP, int puerto, char *recurso)
{
    int socketid; 
    static struct sockaddr_in serverdir; 
    char HTTP_request[8000]; 
    char HTTP_response[40000];

    socketid = socket(AF_INET, SOCK_STREAM, 0);
    if (socketid < 0) {
        perror("Socket");
        return; 
    }
    serverdir.sin_family = AF_INET;
    serverdir.sin_addr.s_addr = inet_addr(dirIP);
    serverdir.sin_port = htons((uint16_t)puerto);

    if (connect(socketid, (struct sockaddr *)&serverdir, sizeof(serverdir)) < 0) {
        perror("Connect");
        close(socketid);
        return;
    }

    sprintf(HTTP_request, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", recurso, dirIP);    
    ssize_t bytes_written = write(socketid, HTTP_request, strlen(HTTP_request));
    if (bytes_written < 0) {
        perror("Write");
        close(socketid);
        return;
    }

    ssize_t bytes_read = read(socketid, HTTP_response, sizeof(HTTP_response) -1);
    if (bytes_read < 0) {
        perror("Read");
        close(socketid);
        return;
    }
    HTTP_response[bytes_read] = '\0';

    printf("%s\n", HTTP_response);
    close(socketid);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <IP> <port> <resource>\n", argv[0]);
        return 1;
    }
    
    char *ip = argv[1];
    int port = atoi(argv[2]);
    char *resource = argv[3];
    
    ClienteHTTP(ip, port, resource);
    return 0;
}