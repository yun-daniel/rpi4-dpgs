#include "server_thread_supv.h"

#include <iostream>


SrvThrSupv::SrvThrSupv(DeviceManager& _dev_mgr)
    : dev_mgr(_dev_mgr) {
}

SrvThrSupv::~SrvThrSupv() {
}


bool SrvThrSupv::initialize() {
    std::cout << "[SRV_THR] Start to initialize...\n";

    std::cout << "[SRV_THR] Success: Thread Supervisor initialized\n";
    return true;
}


void SrvThrSupv::start() {
    std::cout << "[SRV_THR] Start Thread Superviosr\n";

    is_running = true;

    thread_dev_mgr = std::thread([this](){
        dev_mgr.run();
    });

}


void SrvThrSupv::stop() {
    std::cout << "[SRV_THR] Thread Supervisor Terminating...\n";

    dev_mgr.stop();

    clear();

    is_running = false;

    std::cout << "[SRV_THR] Thread Supervisor Terminated\n";
}


void SrvThrSupv::clear() {
    std::cout << "[SRV_THR] clear: Cleanning...\n";

    if (thread_dev_mgr.joinable()) {
        std::cout << "[SRV_THR] Joining thread_dev_mgr\n";
        thread_dev_mgr.join();
    }


    std::cout << "[SRV_THR] clear: Cleanning Success\n";
}


bool SrvThrSupv::monitor() {
    if (!is_running) return false;

    return true;
}
