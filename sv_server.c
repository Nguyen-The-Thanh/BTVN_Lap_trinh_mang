#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 256

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Nhap sai cu phap!");
        return 1;
    }

    int port = atoi(argv[1]);
    char *log_file = argv[2];

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
    bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_fd, 5);

    printf("Server listening on port %d...\n", port);

    while (1) {
        client_sock = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);

        // IP cua client
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));

        // Nhan du lieu tu client
        int n = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
        if (n <= 0) {
            close(client_sock);
            continue;
        }

        buffer[n] = '\0';

        // Lay thoi gian hien tai
        time_t now = time(NULL);
        struct tm *t = localtime(&now);

        char time_str[30];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t);

        
        printf("%s %s %s\n", ip, time_str, buffer);

        // Ghi file
        FILE *f = fopen(log_file, "a");
        if (f) {
            fprintf(f, "%s %s %s\n", ip, time_str, buffer);
            fclose(f);
        }

        close(client_sock);
    }

    close(server_fd);
    return 0;
}