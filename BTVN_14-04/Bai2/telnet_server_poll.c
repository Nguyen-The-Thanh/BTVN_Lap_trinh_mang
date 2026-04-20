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
    int state; 
    char user[50];
} Client;

Client clients[MAX_CLIENTS];
struct pollfd fds[MAX_CLIENTS + 1];
int nClients = 0;

// check login
int check_login(char *user, char *pass) {
    FILE *f = fopen("db.txt", "r");
    if (!f) return 0;

    char u[50], p[50];
    while (fscanf(f, "%s %s", u, p) == 2) {
        if (strcmp(user, u) == 0 && strcmp(pass, p) == 0) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

void remove_client(int i) {
    close(clients[i].fd);

    clients[i] = clients[nClients - 1];
    fds[i + 1] = fds[nClients];

    nClients--;
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

    printf("Telnet server running on port %d...\n", PORT);

    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    char buf[BUF_SIZE];

    while (1) {
        int nfds = nClients + 1;

        int ret = poll(fds, nfds, -1);
        if (ret < 0) break;

        // new client
        if (fds[0].revents & POLLIN) {
            int c = accept(server_fd, NULL, NULL);

            if (nClients >= MAX_CLIENTS) {
                close(c);
                continue;
            }

            clients[nClients].fd = c;
            clients[nClients].state = 0;

            fds[nClients + 1].fd = c;
            fds[nClients + 1].events = POLLIN;

            nClients++;

            send(c, "Username: ", strlen("Username: "), 0);
        }

        for (int i = 0; i < nClients; i++) {
            if (fds[i + 1].revents & POLLIN) {

                int ret = recv(clients[i].fd, buf, sizeof(buf) - 1, 0);
                if (ret <= 0) {
                    remove_client(i);
                    i--;
                    continue;
                }

                buf[ret] = 0;

                char *line = strtok(buf, "\r\n");

                while (line != NULL) {

                    if (clients[i].state == 0) {
                        strcpy(clients[i].user, line);
                        clients[i].state = 1;
                        send(clients[i].fd, "Password: ", strlen("Password: "), 0);
                    }
                    else if (clients[i].state == 1) {
                        if (check_login(clients[i].user, line)) {
                            clients[i].state = 2;
                            send(clients[i].fd, "Login success\n$ ", strlen("Login success\n$ "), 0);
                        } else {
                            send(clients[i].fd, "Login failed\n", strlen("Login failed\n"), 0);
                            clients[i].state = 0;
                            send(clients[i].fd, "Username: ", strlen("Username: "), 0);
                        }
                    }
                    else {
                        char cmd[BUF_SIZE + 50];
                        snprintf(cmd, sizeof(cmd), "%s > output.txt 2>&1", line);
                        system(cmd);

                        FILE *f = fopen("output.txt", "r");
                        if (f) {
                            while (fgets(buf, sizeof(buf), f)) {
                                send(clients[i].fd, buf, strlen(buf), 0);
                            }
                            fclose(f);
                        }

                        send(clients[i].fd, "\n$ ", strlen("\n$ "), 0);
                    }

                    line = strtok(NULL, "\r\n");
                }
            }
        }
    }

    close(server_fd);
    return 0;
}