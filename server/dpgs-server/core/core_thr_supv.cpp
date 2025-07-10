#include "core_thr_supv.h"
#include "vp_engine_dummy.h"

#include <iostream>


CoreThrSupv::CoreThrSupv(FrameBuffer& _fb, MapManager& _map_mgr)
    : fb(_fb), map_mgr(_map_mgr) {
}

CoreThrSupv::~CoreThrSupv() {
}


bool CoreThrSupv::initialize() {
    std::cout << "[THR_SUPV] Start to initialize...\n";

    vp_engine = new VPEngine("tb/test_a.mp4", fb);

    std::cout << "[THR_SUPV] Success: Thread Supervisor initialized\n";
    return true;
}



void CoreThrSupv::start() {
    std::cout << "[THR_SUPV] Start Thread Supervisor\n";

    thread_vpe = std::thread([this]() {
        vp_engine->run();
    });
        

}


void CoreThrSupv::stop() {
    std::cout << "[THR_SUPV] Thread Supervisor Terminating...\n";

    if (vp_engine) vp_engine->stop();


    clear();

    std::cout << "[THR_SUPV] Thread Supervisor Terminated\n";
}


void CoreThrSupv::clear() {
    std::cout << "[THR_SUPV] clear: Cleanning...\n";

    if (thread_vpe.joinable()) {
        std::cout << "[THR_SUPV] Joining thread_vpe\n";
        thread_vpe.join();
    }
    delete vp_engine;
    vp_engine = nullptr;


    std::cout << "[THR_SUPV] clear: Cleanning Success\n";
}

