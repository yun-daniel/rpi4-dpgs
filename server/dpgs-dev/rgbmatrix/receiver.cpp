#include <iostream>
#include <string>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

#define LISTEN_PORT 8888
#define DEVICE_PATH "/dev/rgbmatrix"

int openDevice() {
    int fd = open(DEVICE_PATH, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open Device File\n");
    }
    return fd;
}

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
	perror("Failed to create Socket\n");
        return 1;
    }

    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(LISTEN_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
	perror("Failed to bind socket\n");
        close(sock);
        return 1;
    }

    char buffer[1024];
    int device_fd = openDevice();
    if (device_fd < 0) {
        close(sock);
        return 1;
    }

    // Initialize LED Display
    char cmd[16];
    for (int i=0; i<29; ++i) {
        int len = snprintf(cmd, sizeof(cmd), "%d %d", i, 1);
        std::cout << "[INIT] " << cmd << "\n";
        ssize_t written = write(device_fd, cmd, len);
        if (written < 0) {
            perror("Failed to write Device File\n");
        }
    }


    while (true) {
        ssize_t recv_len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, nullptr, nullptr);
        if (recv_len > 0) {
            buffer[recv_len] = '\0';
            std::cout << "[RECV] " << buffer << "\n";

            ssize_t written = write(device_fd, buffer, recv_len);
            if (written < 0) {
                perror("Failed to write Device File\n");
            }
        }
    }

    close(device_fd);
    close(sock);
    return 0;
}
