#include "cam_thr_supv.h"

#include <iostream>

CamThrSupv::CamThrSupv(VPEngine& _vp_engine, SrvNet& _srv_net)
    : vp_engine(_vp_engine), srv_net(_srv_net) {
}

CamThrSupv::~CamThrSupv() {
}



bool CamThrSupv::initialize() {
    std::cout << "[CAM_THR] Start to initialize...\n";


    std::cout << "[CAM_THR] Success: Thread Supervisor initialized\n";
    return true;
}


void CamThrSupv::start() {
    std::cout << "[CAM_THR] Start Thread Supervisor\n";

    is_running = true;

    thread_vp_engine = std::thread([this](){
        vp_engine.run();
    });

    thread_srv_net = std::thread([this](){
        srv_net.run();
    });

}


void CamThrSupv::stop() {
    std::cout << "[CAM_THR] Thread Supervisor Terminating...\n";

    srv_net.stop();
    vp_engine.stop();

    clear();

    is_running = false;

    std::cout << "[CAM_THR] Thread Supervisor Terminated\n";
}

void CamThrSupv::clear() {
    std::cout << "[CAM_THR] clear: Cleanning...\n";

    if (thread_srv_net.joinable()) {
        std::cout << "[CAM_THR] Joining thread_srv_net\n";
        thread_srv_net.join();
    }

    if (thread_vp_engine.joinable()) {
        std::cout << "[CAM_THR] Joining thread_vp_engine\n";
        thread_vp_engine.join();
    }

    std::cout << "[CAM_THR] clear: Cleanning Success\n";
}


bool CamThrSupv::monitor() {
    if (!is_running) return false;

    return true;
}
