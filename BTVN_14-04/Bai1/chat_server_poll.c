#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>

#define PORT 8888
#define MAX_CLIENTS 100
#define BUF_SIZE 1024

typedef struct {
    int fd;
    int registered; // 0: chưa đăng ký, 1: rồi
    char id[50];
} Client;

Client clients[MAX_CLIENTS];
struct pollfd fds[MAX_CLIENTS + 1];
int nClients = 0;

void remove_client(int i) {
    close(clients[i].fd);

    clients[i] = clients[nClients - 1];
    fds[i + 1] = fds[nClients];

    nClients--;
}

int parse_client_info(char *line, char *id) {
    // format: id: name
    char *colon = strchr(line, ':');
    if (!colon) return 0;

    *colon = 0;
    char *name = colon + 1;

    // bỏ khoảng trắng
    while (*name == ' ') name++;

    if (strlen(line) == 0 || strlen(name) == 0)
        return 0;

    strcpy(id, line);
    return 1;
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 5);

    printf("Chat server running on port %d...\n", PORT);

    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    char buf[BUF_SIZE];

    while (1) {
        int nfds = nClients + 1;

        int ret = poll(fds, nfds, -1);
        if (ret < 0) break;

        // client mới
        if (fds[0].revents & POLLIN) {
            int c = accept(server_fd, NULL, NULL);

            if (nClients >= MAX_CLIENTS) {
                close(c);
                continue;
            }

            clients[nClients].fd = c;
            clients[nClients].registered = 0;

            fds[nClients + 1].fd = c;
            fds[nClients + 1].events = POLLIN;

            nClients++;

            send(c, "Nhap: client_id: client_name\n", 
                 strlen("Nhap: client_id: client_name\n"), 0);
        }

        // xử lý client
        for (int i = 0; i < nClients; i++) {
            if (fds[i + 1].revents & POLLIN) {

                int ret = recv(clients[i].fd, buf, sizeof(buf)-1, 0);
                if (ret <= 0) {
                    remove_client(i);
                    i--;
                    continue;
                }

                buf[ret] = 0;

                // xử lý nhiều dòng
                char *line = strtok(buf, "\r\n");

                while (line != NULL) {

                    if (!clients[i].registered) {
                        char id[50];

                        if (parse_client_info(line, id)) {
                            strcpy(clients[i].id, id);
                            clients[i].registered = 1;

                            send(clients[i].fd, "OK\n", 3, 0);
                        } else {
                            send(clients[i].fd, "Sai format!\n", 12, 0);
                        }
                    } 
                    else {
                        // broadcast
                        char msg[BUF_SIZE];
                        snprintf(msg, sizeof(msg), "%s: %s\n",
                                 clients[i].id, line);

                        for (int j = 0; j < nClients; j++) {
                            if (j != i) {
                                send(clients[j].fd, msg, strlen(msg), 0);
                            }
                        }
                    }

                    line = strtok(NULL, "\r\n");
                }
            }
        }
    }

    close(server_fd);
    return 0;
}