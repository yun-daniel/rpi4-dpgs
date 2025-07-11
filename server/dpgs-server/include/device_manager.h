#ifndef __DEVICE_MANAGER_H__
#define __DEVICE_MANAGER_H__

#include "map_manager.h"
#include "remote_led_dp.h"

#include <sys/types.h>
#include <thread>
#include <atomic>

#define LOCAL_LED_DP    false
#define REMOTE_LED_DP   true


class DeviceManager {
 public:
    DeviceManager(MapManager& _map_mgr);
    ~DeviceManager();

    bool initialize();
    void run();
    void stop();

 private:
    std::atomic<bool>   is_running;
    bool        enabled_rldp = false;

    RemoteLedDP*    rldp = nullptr;

    std::thread     thread_rldp;


    void clear();


    // External Interface
    MapManager&         map_mgr;

};



#endif // __DEVICE_MANAGER_H__
