#ifndef __CLIENT_MANAGER_H__
#define __CLIENT_MANAGER_H__

#include "map_manager.h"
#include "frame_buffer_str.h"

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
#include <atomic>

#include "client_info.h"
#include "srv_sock_utils.h"
#include "streaming_module.h"


class ClientManager {
 public:
    ClientManager(MapManager& _map_mgr, FrameBufferStr& _clt_fb1, FrameBufferStr& _clt_fb2, int _port);
//    ~ClientManager();

    bool initialize();
    void run();
    void stop();

    // For Internal Threads
    std::vector<ClientInfo>::iterator find_client(pthread_t tid);
    static void remove(void *arg);              // tid_vec[2], client_tid_vec
    static void clear();                        // client_tid_vec, attr
    static void *client_thread(void *arg);      // clnt_sock, map data
    static void *send_mapdata(void *arg);
    static void *rtsp(void *arg);
    static void *check_map_update(void *arg);   // mapdata

    static void set_cm(ClientManager *ptr);

 private:
    std::atomic<bool> is_running = false;
    // Network
    int listen_fd, port;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Thread Management
    std::vector<ClientInfo> client_info_vec;
    pthread_attr_t  attr;                   // Thread attributes for detach state
    pthread_mutex_t m_client_info_vec;      // Mutex for client_info_vec and cond_clear
    pthread_mutex_t m_updated;              // Mutex for updated
    pthread_cond_t  cond_clear;             // Signaled when client_info_vec is empty; used to wait until all clients are removed
    pthread_cond_t  cond_updated;           // Broadcasted when the mapdata is updated
    pthread_cond_t  cond_all_sent;           // Signaled when targets all send map

    // Internal Functions
//    void clear();

    // Internal Interface
    static ClientManager* cm_ptr;          // Static instance pointer for static member function
    int mapdata;
    bool updated;                           // Flag for mapdata update

    // External Interface
    MapManager&     map_mgr;
    FrameBufferStr& clt_fb1;
    FrameBufferStr& clt_fb2;


};



#endif // __CLIENT_MANAGER_H__
