#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H

#include <cstdio>
#include <cstdlib>

#include <pthread.h>

#include "connection_manager.h"
#include "map_monitor.h"
#include "static_function_args.h"

class ClientManager
{
private:

    bool is_running;

    int mapdata;
    bool is_updated;
    pthread_mutex_t updated_mutex;
    pthread_cond_t updated_cv;

    SFA sfa;
    pthread_t tid_arr[2];

    ConnectionManager conn_mgr;
    MapMonitor map_mon;

    // VPEngine& vp_engine;
    // MapManager& map_mgr;

public:
    int initialize ();
    int run ();
    int stop ();
    int clear ();

    int * get_mapdata ();
    bool * get_is_updated ();
    pthread_mutex_t * get_updated_mutex ();
    pthread_cond_t * get_updated_cv ();
};

#endif  // CLIENT_MANAGER_H