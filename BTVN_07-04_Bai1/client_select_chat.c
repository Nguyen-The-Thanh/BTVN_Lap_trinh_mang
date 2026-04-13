#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

int main() {
    int client = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(client, (struct sockaddr*)&addr, sizeof(addr));

    printf("Connected to server\n");

    fd_set fdread;
    char buf[256];

    while (1) {
        FD_ZERO(&fdread);
        FD_SET(0, &fdread); // stdin
        FD_SET(client, &fdread);

        select(client + 1, &fdread, NULL, NULL, NULL);

        // Nhập từ bàn phím
        if (FD_ISSET(0, &fdread)) {
            fgets(buf, sizeof(buf), stdin);
            send(client, buf, strlen(buf), 0);
        }

        // Nhận từ server
        if (FD_ISSET(client, &fdread)) {
            int ret = recv(client, buf, sizeof(buf) - 1, 0);
            if (ret <= 0) break;

            buf[ret] = 0;
            printf("%s", buf);
        }
    }

    close(client);
}