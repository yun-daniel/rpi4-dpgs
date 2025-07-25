#ifndef CLIENT_INFO_H
#define CLIENT_INFO_H

#include <unistd.h>
#include <openssl/ssl.h>
#include <pthread.h>

class ClientInfo
{
private:
    bool sent_map_flag;
    int sock_fd;
    pthread_t tid;
    SSL * ssl;

public:
    ClientInfo ();
    bool get_sent_map_flag ();
    int get_sock_fd ();
    pthread_t get_tid ();
    SSL * get_ssl ();

    int set_sent_map_flag (bool flag);
    int set_sock_fd (int fd);
    int set_tid (pthread_t tid);
    int set_ssl (SSL * _ssl);
};

#endif  // CLIENT_INFO_H
