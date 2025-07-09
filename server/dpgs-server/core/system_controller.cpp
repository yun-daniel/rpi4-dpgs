#include "system_controller.h"

#include <iostream>
#include <thread>
#include <chrono>


SystemController::SystemController() {
}

SystemController::~SystemController() {
}


bool SystemController::initialize() {
    std::cout << "[SYS] Start to initialize...\n";


    std::cout << "[SYS] Initialize FrameBuffer...\n";
    fb = std::make_unique<FrameBuffer>("FB");
    if (!fb->initialize()) {
        std::cerr << "[SYS] Error: Failed to initialize FrameBuffer\n";
        return 1;
    }
    std::cout << "[SYS] Success: FrameBuffer Initialized\n";


    std::cout << "[SYS] Initialize MapManager...\n";
    map_mgr = std::make_unique<MapManager>("config/map.json");
    if (!map_mgr->initialize()) {
        std::cerr << "[SYS] Error: Failed to initialize MapManager\n";
        return 1;
    }


    std::cout << "[SYS] Initialize Process Supervisor...\n";
    proc_supv = std::make_unique<CoreProcessSupervisor>(*fb, *map_mgr);

    std::cout << "[SYS] Initialize Thread Manager...\n";
    thread_mgr = std::make_unique<CoreThreadManager>(*fb, *map_mgr);


    std::cout << "[SYS] Success: System Initialized\n";
    return true;
}


void SystemController::start() {
    std::cout << "[SYS] Application Start.\n";

    thread_mgr->start();
    proc_supv->start();


    while (true) {
        if (!proc_supv->monitor()) break;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }


}


void SystemController::stop() {
    std::cout << "[SYS] Application Stop.\n";
    std::cout << "[SYS] Cleanning...\n";

    fb->destroyShm();
    map_mgr->destroyShm();


    std::cout << "[SYS] Success: System Clear\n";
}
