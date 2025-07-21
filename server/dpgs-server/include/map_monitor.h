#ifndef MAP_MONITOR_H
#define MAP_MONITOR_H

//#include "client_manager.h"
#include "srv_sock_utils.h"
#include "static_function_args.h"
#include "map_manager.h"

#include <vector>
#include <algorithm>
#include <pthread.h>
#include <atomic>


class MapMonitor {
 public:
    bool initialize(ClientManager* _clt_mgr_ptr, ConnectionManager* _conn_mgr_ptr, MapManager* _map_mgr);
    void stop ();

    void run();


 private:
    std::atomic<bool> is_running = false;


    // External Interface
    ClientManager*      clnt_mgr_ptr = nullptr;
    ConnectionManager*  conn_mgr_ptr = nullptr;
    MapManager*         map_mgr;


};

#endif  // MAP_MONITOR_H
