#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

#define MAX_CLIENTS 100

typedef struct {
    int fd;
    char id[32];
    char name[32];
    int identified;
} Client;

Client clients[MAX_CLIENTS];
int nClients = 0;

void removeClient(int i) {
    close(clients[i].fd);
    if (i < nClients - 1)
        clients[i] = clients[nClients - 1];
    nClients--;
}

void broadcast(int sender, char *msg) {
    for (int i = 0; i < nClients; i++) {
        if (i != sender && clients[i].identified) {
            send(clients[i].fd, msg, strlen(msg), 0);
        }
    }
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(listener, (struct sockaddr*)&addr, sizeof(addr));
    listen(listener, 5);

    printf("Server listening on 8080...\n");

    fd_set fdread;
    char buf[256];

    while (1) {
        FD_ZERO(&fdread);
        FD_SET(listener, &fdread);
        int maxfd = listener;

        for (int i = 0; i < nClients; i++) {
            FD_SET(clients[i].fd, &fdread);
            if (clients[i].fd > maxfd)
                maxfd = clients[i].fd;
        }

        select(maxfd + 1, &fdread, NULL, NULL, NULL);

        // New connection
        if (FD_ISSET(listener, &fdread)) {
            int client = accept(listener, NULL, NULL);

            if (nClients < MAX_CLIENTS) {
                clients[nClients].fd = client;
                clients[nClients].identified = 0;
                nClients++;

                char *msg = "Nhap: client_id: client_name\n";
                send(client, msg, strlen(msg), 0);

                printf("New client: %d\n", client);
            } else {
                close(client);
            }
        }

        // Handle clients
        for (int i = 0; i < nClients; i++) {
            if (FD_ISSET(clients[i].fd, &fdread)) {
                int ret = recv(clients[i].fd, buf, sizeof(buf) - 1, 0);
                if (ret <= 0) {
                    printf("Client %d disconnected\n", clients[i].fd);
                    removeClient(i);
                    i--;
                    continue;
                }

                buf[ret] = 0;

                // Nếu chưa đăng ký
                if (!clients[i].identified) {
                    char *p = strstr(buf, ":");
                    if (p) {
                        *p = 0;
                        strcpy(clients[i].id, buf);
                        strcpy(clients[i].name, p + 1);
                        clients[i].name[strcspn(clients[i].name, "\n")] = 0;

                        clients[i].identified = 1;

                        char msg[256];
                        sprintf(msg, "Welcome %s\n", clients[i].name);
                        send(clients[i].fd, msg, strlen(msg), 0);

                        printf("Client registered: %s (%s)\n",
                               clients[i].id, clients[i].name);
                    } else {
                        char *err = "Sai format. Nhap lai!\n";
                        send(clients[i].fd, err, strlen(err), 0);
                    }
                } 
                // Nếu đã đăng ký → broadcast
                else {
                    char msg[300];
                    sprintf(msg, "%s: %s", clients[i].id, buf);
                    broadcast(i, msg);
                    printf("%s", msg);
                }
            }
        }
    }

    close(listener);
}