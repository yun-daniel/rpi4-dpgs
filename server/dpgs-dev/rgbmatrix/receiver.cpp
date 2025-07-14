#include <iostream>
#include <string>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

#define LISTEN_PORT 8888
#define DEVICE_PATH "/dev/rgbmatrix"

// 디바이스 파일 열기
int openDevice() {
    int fd = open(DEVICE_PATH, O_WRONLY);
    if (fd < 0) {
        perror("디바이스 열기 실패");
    }
    return fd;
}

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("소켓 생성 실패");
        return 1;
    }

    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(LISTEN_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind 실패");
        close(sock);
        return 1;
    }

    char buffer[1024];
    int device_fd = openDevice();
    if (device_fd < 0) {
        close(sock);
        return 1;
    }

    while (true) {
        ssize_t recv_len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, nullptr, nullptr);
        if (recv_len > 0) {
            buffer[recv_len] = '\0';
            std::cout << "[RECV] " << buffer << "\n";

            ssize_t written = write(device_fd, buffer, recv_len);
            if (written < 0) {
                perror("디바이스 쓰기 실패");
            }
        }
    }

    close(device_fd);
    close(sock);
    return 0;
}
