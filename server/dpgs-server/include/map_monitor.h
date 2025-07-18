#ifndef MAP_MONITOR_H
#define MAP_MONITOR_H

#include <pthread.h>

#include "static_function_args.h"

class MapMonitor
{
private:

public:
    int initialize ();
    int stop ();

    static void * run (void * arg);
};

#endif  // MAP_MONITOR_H