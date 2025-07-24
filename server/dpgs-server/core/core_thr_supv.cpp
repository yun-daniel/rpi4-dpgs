#include "core_thr_supv.h"
#include "vp_engine.h"

#include <iostream>


CoreThrSupv::CoreThrSupv(FrameBuffer& _fb, MapManager& _map_mgr)
    : fb(_fb), map_mgr(_map_mgr) {
}

CoreThrSupv::~CoreThrSupv() {
}


bool CoreThrSupv::initialize() {
    std::cout << "[THR_SUPV] Start to initialize...\n";

    vp_engine = new VPEngine(fb);
    if (!vp_engine->initialize()) {
        std::cerr << "[THR_SUPV] Error: Failed to initialzie Video Processing Engine\n";
        return false;
    }

    dev_mgr = new DeviceManager(map_mgr);
    if (!dev_mgr->initialize()) {
        std::cerr << "[THR_SUPV] Error: Failed to initialize Device Manager\n";
        return false;
    }

    clt_mgr = new ClientManager(map_mgr, *vp_engine);
    if (!clt_mgr->initialize()) {
        std::cerr << "[THR_SUPV] Error: Failed to initialize Client Manager\n";
        return false;
    }


    std::cout << "[THR_SUPV] Success: Thread Supervisor initialized\n";
    return true;
}



void CoreThrSupv::start() {
    std::cout << "[THR_SUPV] Start Thread Supervisor\n";

    is_running = true;

    thread_vpe = std::thread([this]() {
        vp_engine->run();
    });

    thread_dev = std::thread([this]() {
        dev_mgr->run();
    });

    thread_clt = std::thread([this]() {
        clt_mgr->run();
    });

}


void CoreThrSupv::stop() {
    std::cout << "[THR_SUPV] Thread Supervisor Terminating...\n";

    if (vp_engine) vp_engine->stop();
    if (dev_mgr) dev_mgr->stop();
    if (clt_mgr) clt_mgr->stop();

    clear();

    is_running = false;

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

    if (thread_dev.joinable()) {
        std::cout << "[THR_SUPV] Joining thread_dev\n";
        thread_dev.join();
    }
    delete dev_mgr;
    dev_mgr = nullptr;

    if (thread_clt.joinable()) {
        std::cout << "[THR_SUPV] Joining thread_clt\n";
        thread_clt.join();
    }
    delete clt_mgr;
    clt_mgr = nullptr;


    std::cout << "[THR_SUPV] clear: Cleanning Success\n";
}


bool CoreThrSupv::monitor() {
    if (!is_running) return false;
    if (!vp_engine->is_run()) return false;

    return true;
}
