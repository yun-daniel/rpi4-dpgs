#ifndef __DEVICE_MANAGER_H__
#define __DEVICE_MANAGER_H__

#include "config.h"
#include "map_manager.h"
#include "remote_led_dp.h"

#include <sys/types.h>
#include <thread>
#include <atomic>


class DeviceManager {
 public:
    DeviceManager(MapManager& _map_mgr);
    ~DeviceManager();

    bool initialize();
    void run();
    void stop();

 private:
    std::atomic<bool> is_running;
    bool enable_rldp = false;

    RemoteLedDP*    rldp = nullptr;

    std::thread     thread_rldp;


    void clear();


    // External Interface
    MapManager&         map_mgr;

};



#endif // __DEVICE_MANAGER_H__
