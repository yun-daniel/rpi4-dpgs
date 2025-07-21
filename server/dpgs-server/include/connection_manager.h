#ifndef __CONNECTION_MANAGER_H__
#define __CONNECTION_MANAGER_H__

#include "client_info.h"
#include "srv_sock_utils.h"
#include "static_function_args.h"
#include "streaming_module.h"
#include "vp_engine.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <atomic>


using namespace std;


class ConnectionManager {
 public:
    bool initialize(ClientManager* _clt_mgr, VPEngine* _vp_engine);
    void stop ();

    vector<ClientInfo>::iterator find_client (pthread_t tid);

    void run();
    void exec_client_thread(int clnt_sock);

    // Interface
    std::vector<ClientInfo>* get_client_info_vec();
    pthread_mutex_t* get_client_info_vec_mutex();
    pthread_cond_t* get_all_sent_cv();


    // Thread Handler
    static void* handle_client_thread(void* arg);
    
//    static void * run (void * arg);
//    static void * client_thread (void * arg);
    static void remove (void * arg);
    static void * send_mapdata (void * arg);
    static void * streaming (void * arg);


 private:
    std::atomic<bool> is_running = false;
    int listen_fd, port;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    vector<ClientInfo> client_info_vec;

    pthread_attr_t attr;
    pthread_mutex_t client_info_vec_mutex;
    pthread_cond_t empty_cv;
    pthread_cond_t all_sent_cv;

    void clear();


    // External Interface
    ClientManager*  clt_mgr_ptr = nullptr;
    VPEngine*       vp_engine_ptr = nullptr;


};

#endif  // __CONNECTION_MANAGER_H__
