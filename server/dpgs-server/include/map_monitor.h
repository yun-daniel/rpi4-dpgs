#ifndef MAP_MONITOR_H
#define MAP_MONITOR_H

#include <pthread.h>

#include "map_manager.h"
#include "static_function_args.h"

class MapMonitor
{
private:

public:
    bool initialize (MapManager& _map_mgr);
    void stop ();

    static void * run (void * arg);
};

#endif  // MAP_MONITOR_H
