#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

int procesar(char *HTTP_request, char *HTTP_response, size_t resp_size) {

    char *resource = HTTP_request + 4;

    char *space = resource;
    while (*space != ' ' && *space != '\0') {
        space++;
    }

    if (*space == ' ') {
        *space = '\0';
    }

    if (strcmp(resource, "/") == 0) strcpy(resource, "/index.html");
    
    char filepath[256];
    if (resource[0] == '/') {
        snprintf(filepath, sizeof(filepath), ".%s", resource);
    } else {
        snprintf(filepath, sizeof(filepath), "./%s", resource);
    }

    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        snprintf(HTTP_response, resp_size,
            "HTTP/1.1 404 Not Found\r\n"
            "Connection: close\r\n"
            "\r\n"
            "404 Not Found");
        return -1;
    }

    char file_content[resp_size - 512];
    ssize_t len = read(fd, file_content, sizeof(file_content));
    close(fd);

    int hlen = snprintf(HTTP_response, resp_size, 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %zd\r\n"
        "Connection: close\r\n"
        "\r\n", len);
    
    if ((size_t)hlen + len < resp_size) {
        memcpy(HTTP_response + hlen, file_content, len);
        HTTP_response[hlen + len] = '\0';

    } else {
        snprintf(HTTP_response, resp_size, 
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Connection: close\r\n"
            "\r\n"
            "500 Response too large");
        return -1;
    }
    return 0;
}

void ServidorHTTP(char *dirIP, int puerto){
    int socketid;
    static struct sockaddr_in serverdir;

    serverdir.sin_family = AF_INET;
    serverdir.sin_addr.s_addr = inet_addr(dirIP);
    serverdir.sin_port = htons((uint16_t)puerto);

    socketid = socket(AF_INET, SOCK_STREAM, 0);
    if (socketid < 0) {
        perror("Socket");
        exit(1);
    }
    if (bind(socketid, (struct sockaddr *)&serverdir, sizeof(serverdir)) < 0) {
        perror("Bind");
        close(socketid);
        exit(1);
    }
    if (listen(socketid, 10) < 0) {
        perror("Listen");
        close(socketid);
        exit(1);
    }

    while(1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        char HTTP_request[8000];
        char HTTP_response[40000];

        int socketclienteid = accept(socketid, (struct sockaddr *)&client_addr, &client_len);
        if (socketclienteid < 0) {
            perror("Accept");
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork");
            close(socketclienteid);
        }
        else if(pid == 0) {
            close(socketid);
            ssize_t bytes_read= read(socketclienteid, HTTP_request, sizeof(HTTP_request) -1);
            if (bytes_read > 0) {
                HTTP_request[bytes_read] = '\0';
                procesar(HTTP_request, HTTP_response, sizeof(HTTP_response));
                write(socketclienteid, HTTP_response, strlen(HTTP_response));
            }
            close(socketclienteid);
            exit(0);
        }
        else {
            close(socketclienteid);
        }
    }
}

int main() {
    ServidorHTTP("127.0.0.1", 9999);
    return 0;
}
