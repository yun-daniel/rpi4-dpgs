#ifndef __SERVER_THREAD_SUPERVISOR_H__
#define __SERVER_THREAD_SUPERVISOR_H__

#include "device_manager.h"

#include <sys/types.h>
#include <cstdio>
#include <thread>


class SrvThrSupv {
 public:
    SrvThrSupv(DeviceManager& _dev_mgr);
    ~SrvThrSupv();

    bool initialize();
    void start();
    void stop();

    bool monitor();

 private:
    bool is_running = false;

    DeviceManager&   dev_mgr;

    std::thread thread_dev_mgr;

    void clear();


};




#endif // __SERVER_THREAD_SUPERVISOR_H__
