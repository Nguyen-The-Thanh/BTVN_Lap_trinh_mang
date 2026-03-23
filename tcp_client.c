#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 256

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Nhap sai cu phap!");
        return 1;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);

    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Tao socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket error");
        return 1;
    }

    // Cau hinh dia chi server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    // Ket noi den server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect failed");
        return 1;
    }

    printf("Connected to server!\n");

    // Nhan greeting tu server
    int n = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (n > 0) {
        buffer[n] = '\0';
        printf("Server: %s\n", buffer);
    }

    // Nhap va gui du lieu den server tu ban phim
    while (1) {
        printf("Enter message: ");
        fgets(buffer, BUFFER_SIZE, stdin);

        if (strncmp(buffer, "exit", 4) == 0) break;

        send(sock, buffer, strlen(buffer), 0);
    }

    close(sock);
    return 0;
}