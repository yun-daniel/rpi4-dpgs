#include "dpgs_server.h"

#include <iostream>
#include <thread>
#include <chrono>


DPGSServer::DPGSServer() {
}

DPGSServer::~DPGSServer() {
}


bool DPGSServer::initialize() {
    std::cout << "[SYS] Start to initialize...\n";


    std::cout << "[SYS] Initialize StrFrameBuffer...\n";
    fb = std::make_unique<StrFrameBuffer>();
    if (!fb->initialize()) {
        std::cerr << "[SYS] Error: Failed to initialize StrFrameBuffer\n";
        return false;
    }
    std::cout << "[SYS] Success: StrFrameBuffer Initialized\n";


    std::cout << "[SYS] Initialize Device Manager\n";
    dev_mgr = std::make_unique<DeviceManager>(*fb);
    if (!dev_mgr->initialize()) {
        std::cerr << "[SYS] Error: Failed to initialize Device Manager\n";
        return false;
    }
    std::cout << "[SYS] Success: Device Manager Initialized\n";


    std::cout << "[SYS] Initialize Thread Supervisor\n";
    thr_supv = std::make_unique<SrvThrSupv>(*dev_mgr);
    if (!thr_supv->initialize()) {
        std::cerr << "[SYS] Error: Failed to initialize Thread Supervisor\n";
        return false;
    }


    std::cout << "[SYS] Success: System Initialized\n";
    return true;
}


void DPGSServer::start() {
    std::cout << "[SYS] Application Start.\n";

    thr_supv->start();

    while (thr_supv->monitor()) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}


void DPGSServer::stop() {
    std::cout << "[SYS] Application Terminating...\n";

    thr_supv->stop();

    std::cout << "[SYS] Application Terminated\n";
}


void DPGSServer::clear() {
    std::cout << "[SYS] clear: Cleanning...\n";

    thr_supv.reset();
    dev_mgr.reset();
    fb.reset();

    std::cout << "[SYS] clear: Cleanning Success\n";
}

