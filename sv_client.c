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

    // Nhap du lieu
    char mssv[20], name[100], ngaysinh[20];
    float gpa;

    printf("MSSV: ");
    scanf("%s", mssv);
    getchar();

    printf("Ho ten: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;

    printf("Ngay sinh (YYYY-MM-DD): ");
    scanf("%s", ngaysinh);

    printf("GPA: ");
    scanf("%f", &gpa);

    // Dong goi du lieu
    snprintf(buffer, BUFFER_SIZE, "%s %s %s %.2f", mssv, name, ngaysinh, gpa);

    // Gui du lieu den server
    send(sock, buffer, strlen(buffer), 0);

    printf("Sent!\n");

    close(sock);
    return 0;
}