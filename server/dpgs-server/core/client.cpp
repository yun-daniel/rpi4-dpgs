#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    const char* SERVER_IP   = "127.0.0.1";
    const int   SERVER_PORT = 9999;
    int sockfd;
    struct sockaddr_in serv_addr;

    // IDs and passwords are fixed-size 16-byte buffers
    char id[16] = {0};
    char pw[16] = {0};

    // copy your credentials
    strncpy(id, "jennie", sizeof(id)-1);  // leave room for null-terminator
    strncpy(pw, "1234",   sizeof(pw)-1);

    // 1) create a TCP socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    // 2) set up the server address struct
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        return EXIT_FAILURE;
    }

    // 3) connect to the server
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return EXIT_FAILURE;
    }

    // 4) send ID and password (each exactly 16 bytes)
    if (send(sockfd, id, sizeof(id), 0) != sizeof(id)) {
        perror("send id");
    }
    if (send(sockfd, pw, sizeof(pw), 0) != sizeof(pw)) {
        perror("send pw");
    }

    // 5) receive 1-byte authentication reply
    char reply;
    ssize_t n = recv(sockfd, &reply, 1, 0);
    if (n < 0) {
        perror("recv");
    } else if (n == 0) {
        std::cerr << "server closed connection\n";
    } else {
        if (reply == '0')
            std::cout << "auth fail\n";
        else if (reply == '1')
            std::cout << "auth success\n";
        else
            std::cout << "unknown reply: " << static_cast<int>(reply) << "\n";
    }

    // 6) get a single character from user and send it
    char input = 0x0;
    while (input != '3') {
        printf("Enter a character to send: ");
        scanf(" %c", &input);
        if (send(sockfd, &input, 1, 0) != 1) {
            perror("send input");
        }
    }
    
    // 7) clean up
    close(sockfd);
    return 0;
}
