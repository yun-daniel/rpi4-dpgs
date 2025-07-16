#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>

#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include <time.h>

// #include "map_manager.h"
#include "client_info.h"
#include "streaming_module.hpp"
#include "vp_engine.hpp"
#include "srv_sock_utils.h"

using namespace std;

class ClientManager 
{
private:
    int listen_fd, port;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    vector <ClientInfo> client_info_vec;

    // MapManager&  map_mgr;
    // mapdata
    int mapdata;
    bool updated;

    pthread_attr_t attr;                    // Thread attributes for detach state                            
    pthread_mutex_t m_client_info_vec;      // Mutex for client_info_vec
    pthread_mutex_t m_updated;
    pthread_cond_t cond_clear;
    pthread_cond_t cond_updated;
    pthread_cond_t cond_all_sent;

    static ClientManager * cm_ptr;          // Static instance pointer for static member function

public:
    ClientManager(int port);
    // ClientManager(MapManager& map_mgr);
    int initialize ();
    void connect_client ();
    vector<ClientInfo>::iterator find_client (pthread_t tid);
    
    static void remove (void * arg);            // tid_vec[2], client_tid_vec
    static void clear ();                       // client_tid_vec, attr
    static void * client_thread (void * arg);   // clnt_sock, map data
    static void * check_map_update (void * arg);// mapdata
    //static void unlock_mutex (void * arg);
    static void * send_mapdata (void * arg);
    static void * rtsp (void * arg);

    static void set_cm (ClientManager * ptr);
};

#endif  // CLIENT_MANAGER_H