#include "device_manager.h"

#include <iostream>


DeviceManager::DeviceManager(MapManager& _map_mgr)
    : map_mgr(_map_mgr) {

    enabled_rldp = REMOTE_LED_DP;

}

DeviceManager::~DeviceManager() {
}


bool DeviceManager::initialize() {
    std::cout << "[DEV] Start to initialize...\n";


    if (enabled_rldp) {
        rldp = new RemoteLedDP(map_mgr);
        if (!rldp->initialize()) {
            std::cerr << "[DEV] Error: Failed to initialize Remoted LED Display\n";
            return false;
        }
    }


    std::cout << "[DEV] Success: Device Manager initialized\n";
    return true;
}


void DeviceManager::run() {
    std::cout << "[DEV] Start Device Manager\n";

    thread_rldp = std::thread([this]() {
        rldp->run();
    });

}


void DeviceManager::stop() {
    std::cout << "[DEV] Device Manager Terminating...\n";

    if (rldp) rldp->stop();

    clear();

    std::cout << "[DEV] Device Manager Terminated\n";
}


void DeviceManager::clear() {
    std::cout << "[DEV] clear: Cleanning...\n";

    if (thread_rldp.joinable()) {
        std::cout << "[DEV] Joining thread_rldp\n";
        thread_rldp.join();
    }
    delete rldp;
    rldp = nullptr;


    std::cout << "[DEV] clear: Cleanning Success\n";
}
