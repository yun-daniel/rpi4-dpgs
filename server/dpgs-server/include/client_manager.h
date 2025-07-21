#ifndef __CLIENT_MANAGER_H__
#define __CLIENT_MANAGER_H__

#include "map_manager.h"
#include "vp_engine.h"

#include "connection_manager.h"
#include "map_monitor.h"
#include "static_function_args.h"

#include <cstdio>
#include <cstdlib>
#include <pthread.h>


class ClientManager {
 public:
    ClientManager(MapManager& _map_mgr, VPEngine& _vp_engine);

    bool initialize();
    void run();
    void stop();

    SharedParkingLotMap*    get_mapdata();
    bool*                   get_is_updated();
    pthread_mutex_t*        get_updated_mutex();
    pthread_cond_t*         get_updated_cv();

    ConnectionManager conn_mgr;
    MapMonitor map_mon;

    // Thread Handler
    static void* handle_ConnMgr_run(void* arg);
    static void* handle_MapMon_run(void* arg);


 private:
    std::atomic<bool> is_running = false;

    SharedParkingLotMap mapdata;
    bool            is_updated;
    pthread_mutex_t updated_mutex;
    pthread_cond_t  updated_cv;

    SFA sfa;
    pthread_t tid_arr[2];

    void clear();


    // External Interface
    MapManager&     map_mgr;
    VPEngine&	    vp_engine;

};



#endif // __CLIENT_MANAGER_H__
