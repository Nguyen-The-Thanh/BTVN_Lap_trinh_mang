// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <arpa/inet.h>
// #include <sys/socket.h>
// #include <sys/select.h>

// #define MAX_CLIENTS 100

// typedef struct {
//     int fd;
//     int state;
//     char user[32];
// } Client;

// Client clients[MAX_CLIENTS];
// int nClients = 0;

// int checkLogin(char *user, char *pass) {
//     FILE *f = fopen("telnet.txt", "r");
//     if (!f) return 0;

//     char u[32], p[32];
//     while (fscanf(f, "%s %s", u, p) == 2) {
//         if (strcmp(user, u) == 0 && strcmp(pass, p) == 0) {
//             fclose(f);
//             return 1;
//         }
//     }
//     fclose(f);
//     return 0;
// }

// void removeClient(int i) {
//     close(clients[i].fd);
//     clients[i] = clients[nClients - 1];
//     nClients--;
// }

// int main() {
//     int listener = socket(AF_INET, SOCK_STREAM, 0);

//     struct sockaddr_in addr = {0};
//     addr.sin_family = AF_INET;
//     addr.sin_port = htons(8080);
//     addr.sin_addr.s_addr = INADDR_ANY;

//     bind(listener, (struct sockaddr*)&addr, sizeof(addr));
//     listen(listener, 5);

//     printf("Server running...\n");

//     fd_set fdread;
//     char buf[1024];

//     while (1) {
//         FD_ZERO(&fdread);
//         FD_SET(listener, &fdread);
//         int maxfd = listener;

//         for (int i = 0; i < nClients; i++) {
//             FD_SET(clients[i].fd, &fdread);
//             if (clients[i].fd > maxfd)
//                 maxfd = clients[i].fd;
//         }

//         select(maxfd + 1, &fdread, NULL, NULL, NULL);

//         // accept
//         if (FD_ISSET(listener, &fdread)) {
//             int c = accept(listener, NULL, NULL);

//             clients[nClients].fd = c;
//             clients[nClients].state = 0;
//             nClients++;

//             send(c, "Username: ", strlen("Username: "), 0);
//         }

//         // xử lý client
//         for (int i = 0; i < nClients; i++) {
//             if (FD_ISSET(clients[i].fd, &fdread)) {

//                 int ret = recv(clients[i].fd, buf, sizeof(buf)-1, 0);
//                 if (ret <= 0) {
//                     removeClient(i);
//                     i--;
//                     continue;
//                 }

//                 buf[ret] = 0;

//                 // 🔥 tách từng dòng chuẩn TCP
//                 char *saveptr;
//                 char *line = strtok_r(buf, "\r\n", &saveptr);

//                 while (line != NULL) {

//                     if (clients[i].state == 0) {
//                         strcpy(clients[i].user, line);
//                         clients[i].state = 1;
//                         send(clients[i].fd, "Password: ", strlen("Password: "), 0);
//                     }
//                     else if (clients[i].state == 1) {
//                         if (checkLogin(clients[i].user, line)) {
//                             clients[i].state = 2;
//                             send(clients[i].fd, "Login success\n$ ", strlen("Login success\n$ "), 0);
//                         } else {
//                             send(clients[i].fd, "Login failed\n", strlen("Login failed\n"), 0);
//                             clients[i].state = 0;
//                             send(clients[i].fd, "Username: ", strlen("Username: "), 0);
//                         }
//                     }
//                     else {
//                         char cmd[512];
//                         snprintf(cmd, sizeof(cmd), "%s > out.txt", line);
//                         system(cmd);

//                         FILE *f = fopen("out.txt", "r");
//                         if (f) {
//                             while (fgets(buf, sizeof(buf), f)) {
//                                 send(clients[i].fd, buf, strlen(buf), 0);
//                             }
//                             fclose(f);
//                         }

//                         send(clients[i].fd, "\n$ ", strlen("\n$ "), 0);
//                     }

//                     line = strtok_r(NULL, "\r\n", &saveptr);
//                 }
//             }
//         }
//     }
// }
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

// Hàm kiểm tra đăng nhập từ file text
int check_login(char *user, char *pass) {
    FILE *f = fopen("telnet.txt", "r");
    if (f == NULL) return 0;

    char f_user[50], f_pass[50];
    while (fscanf(f, "%s %s", f_user, f_pass) != EOF) {
        if (strcmp(user, f_user) == 0 && strcmp(pass, f_pass) == 0) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

int main() {
    int server_fd, client_fds[MAX_CLIENTS];
    int authenticated[MAX_CLIENTS]; // Trạng thái đăng nhập của từng client
    struct sockaddr_in server_addr;
    fd_set readfds;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_fds[i] = -1;
        authenticated[i] = 0;
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8888);

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_fd, 5);

    printf("Server đang chạy trên port 8888...\n");

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int max_fd = server_fd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_fds[i] > 0) {
                FD_SET(client_fds[i], &readfds);
                if (client_fds[i] > max_fd) max_fd = client_fds[i];
            }
        }

        select(max_fd + 1, &readfds, NULL, NULL, NULL);

        // Chấp nhận kết nối mới
        if (FD_ISSET(server_fd, &readfds)) {
            int new_socket = accept(server_fd, NULL, NULL);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_fds[i] == -1) {
                    client_fds[i] = new_socket;
                    send(new_socket, "Vui lòng gửi user pass (VD: admin admin):\n", 43, 0);
                    break;
                }
            }
        }

        // Kiểm tra dữ liệu từ các client cũ
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_fds[i] > 0 && FD_ISSET(client_fds[i], &readfds)) {
                char buffer[BUFFER_SIZE];
                int valread = recv(client_fds[i], buffer, BUFFER_SIZE, 0);

                if (valread <= 0) {
                    close(client_fds[i]);
                    client_fds[i] = -1;
                    authenticated[i] = 0;
                } else {
                    buffer[valread] = '\0';
                    // Xóa ký tự xuống dòng nếu có
                    strtok(buffer, "\n\r");

                    if (!authenticated[i]) {
                        // Xử lý đăng nhập
                        char user[50], pass[50];
                        if (sscanf(buffer, "%s %s", user, pass) == 2) {
                            if (check_login(user, pass)) {
                                authenticated[i] = 1;
                                send(client_fds[i], "Đăng nhập thành công! Nhập lệnh:\n", 35, 0);
                            } else {
                                send(client_fds[i], "Sai tài khoản, thử lại:\n", 26, 0);
                            }
                        }
                    } else {
                        // Thực thi lệnh bằng system()
                        char cmd[BUFFER_SIZE + 20];
                        sprintf(cmd, "%s > out.txt", buffer);
                        system(cmd);

                        // Đọc file out.txt và gửi lại kết quả
                        FILE *f = fopen("out.txt", "r");
                        if (f) {
                            while (fgets(buffer, BUFFER_SIZE, f)) {
                                send(client_fds[i], buffer, strlen(buffer), 0);
                            }
                            fclose(f);
                        }
                    }
                }
            }
        }
    }
    return 0;
}