#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 256

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Nhap sai cu phap!");
        return 1;
    }

    int port = atoi(argv[1]);
    char *greeting_file = argv[2];
    char *output_file = argv[3];

    int server_fd, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    char buffer[BUFFER_SIZE];
    // Tao socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket error");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind socket voi dia chi va cong
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    listen(server_fd, 5);
    printf("Server listening on port %d...\n", port);

    while (1) {
        client_sock = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Client connected!\n");

        // Mo file greeting va gui cho client
        FILE *f = fopen(greeting_file, "r");
        if (f) {
            size_t len = fread(buffer, 1, BUFFER_SIZE - 1, f);
            buffer[len] = '\0';
            send(client_sock, buffer, strlen(buffer), 0);
            fclose(f);
        }

        // Nhan du lieu tu client va luu vao file
        FILE *out = fopen(output_file, "a");

        int n;
        while ((n = recv(client_sock, buffer, BUFFER_SIZE - 1, 0)) > 0) {
            buffer[n] = '\0';
            printf("Received: %s", buffer);

            if (out) {
                fprintf(out, "%s", buffer);
                fflush(out);
            }
        }

        if (out) fclose(out);
        close(client_sock);
        printf("Client disconnected.\n");
    }

    close(server_fd);
    return 0;
}