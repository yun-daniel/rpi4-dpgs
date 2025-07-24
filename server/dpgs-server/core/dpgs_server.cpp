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
    fb = std::make_unique<FrameBuffer>(FB_SHM_NAME);
    if (!fb->initialize()) {
        std::cerr << "[SYS] Error: Failed to initialize FrameBuffer\n";
        return false;
    }
    std::cout << "[SYS] Success: FrameBuffer Initialized\n";


    std::cout << "[SYS] Initialize MapManager...\n";
    map_mgr = std::make_unique<MapManager>(MM_SHM_NAME, MM_MAP_FILE_PATH);
    if (!map_mgr->initialize()) {
        std::cerr << "[SYS] Error: Failed to initialize MapManager\n";
        return false;
    }


    std::cout << "[SYS] Success: System Initialized\n";
    return true;
}


bool DPGSServer::start() {
    std::cout << "[SYS] Application Start.\n";

    if(!initialize_proc_supv()) {
        std::cerr << "[SYS] Error: Failed to initialize Process Supervisor\n";
        return false;
    }

 #if ENABLE_AI_ENGINE
    proc_supv->start();
 #endif

    if(!initialize_thr_supv()) {
        std::cerr << "[SYS] Error: Failed to initialize Thread Supervisor\n";
        return false;
    }
    thr_supv->start();

    while (proc_supv->monitor() || thr_supv->monitor()) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return true;
}


void DPGSServer::stop() {
    std::cout << "[SYS] Application Terminating...\n";

    proc_supv->stop();
    thr_supv->stop();

    std::cout << "[SYS] Application Terminated\n";
}


bool DPGSServer::initialize_proc_supv() {
    std::cout << "[SYS] Initialize Process Supervisor...\n";
    proc_supv = std::make_unique<CoreProcSupv>(*fb, *map_mgr);
    if (!proc_supv->initialize()) {
        std::cerr << "[SYS] Error: Failed to initialize CoreProcSupv\n";
        return false;
    }

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

    thr_supv.reset();
    proc_supv.reset();
    map_mgr.reset();
    fb.reset();

    std::cout << "[SYS] clear: Cleanning Success\n";
}

