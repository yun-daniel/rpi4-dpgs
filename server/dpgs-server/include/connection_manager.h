#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "client_info.h"
#include "srv_sock_utils.h"
#include "static_function_args.h"

using namespace std;

class ConnectionManager
{
private:

    int listen_fd, port;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    vector<ClientInfo> client_info_vec;

    pthread_attr_t attr;
    pthread_mutex_t client_info_vec_mutex;
    pthread_cond_t empty_cv;

public:
    int initialize ();
    int stop ();
    int clear ();
    vector<ClientInfo>::iterator find_client (pthread_t tid);

    static void * run (void * arg);
    static void * client_thread (void * arg);
    static void remove (void * arg);
    static void * send_mapdata (void * arg);
    static void * streaming (void * arg);
};

#endif  // CONNECTION_MANAGER_H