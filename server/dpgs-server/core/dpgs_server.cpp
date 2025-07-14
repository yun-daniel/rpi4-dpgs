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


    std::cout << "[SYS] Initialize FrameBuffer...\n";
    fb = std::make_unique<FrameBuffer>("FB");
    if (!fb->initialize()) {
        std::cerr << "[SYS] Error: Failed to initialize FrameBuffer\n";
        return false;
    }
    std::cout << "[SYS] Success: FrameBuffer Initialized\n";


    std::cout << "[SYS] Initialize MapManager...\n";
    map_mgr = std::make_unique<MapManager>("config/map.json");
    if (!map_mgr->initialize()) {
        std::cerr << "[SYS] Error: Failed to initialize MapManager\n";
        return false;
    }


    std::cout << "[SYS] Success: System Initialized\n";
    return true;
}


void DPGSServer::start() {
    std::cout << "[SYS] Application Start.\n";

    if(!initialize_proc_supv()) {
        std::cerr << "[SYS] Error: Failed to initialize Process Supervisor\n";
        return;
    }
    proc_supv->start();

    if(!initialize_thr_supv()) {
        std::cerr << "[SYS] Error: Failed to initialize Thread Supervisor\n";
        return;
    }
    thr_supv->start();

    while (proc_supv->monitor()) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }


}


void DPGSServer::stop() {
    std::cout << "[SYS] Application Terminating...\n";

    proc_supv->stop();
    thr_supv->stop();

    clear();

    std::cout << "[SYS] Application Terminated\n";
}


bool DPGSServer::initialize_proc_supv() {
    std::cout << "[SYS] Initialize Process Supervisor...\n";
    proc_supv = std::make_unique<CoreProcSupv>(*fb, *map_mgr);

    return true;
}

bool DPGSServer::initialize_thr_supv() {
    std::cout << "[SYS] Initialize Thread Supervisor...\n";
    thr_supv = std::make_unique<CoreThrSupv>(*fb, *map_mgr);
    if (!thr_supv->initialize()) {
        std::cerr << "[SYS] Error: Failed to initialize CoreThrSupv\n";
        return false;
    }

    return true;
}


void DPGSServer::clear() {
    std::cout << "[SYS] clear: Cleanning...\n";

    fb->destroyShm();
    map_mgr->destroyShm();

    std::cout << "[SYS] clear: Cleanning Success\n";
}

