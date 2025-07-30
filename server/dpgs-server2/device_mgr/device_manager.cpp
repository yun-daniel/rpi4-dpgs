#include "device_manager.h"

#include <iostream>


DeviceManager::DeviceManager(StrFrameBuffer& _fb)
    : fb(_fb) {

    cam1_en = true;

}

DeviceManager::~DeviceManager() {
}


bool DeviceManager::initialize() {
    std::cout << "[DEV] Start to initialize...\n";

    if (cam1_en) {
        cam1_stream = new CamRxModule(CAM1_SRC_PIPE, fb);
        if (!cam1_stream->initialize()) {
            std::cerr << "[DEV] Error: Failed to initialize Camera1 Stream\n";
            return false;
        }
    }


    std::cout << "[DEV] Success: Device Manager initialized\n";
    return true;
}


void DeviceManager::run() {
    std::cout << "[DEV] Start Device Manager\n";

    if (cam1_en) {
        thread_cam1 = std::thread([this]() {
            cam1_stream->run();
        });
    }

    is_running = true;

}


void DeviceManager::stop() {
    std::cout << "[DEV] Device Manager Terminating...\n";

    if (cam1_stream) cam1_stream->stop();

    clear();

    is_running = false;

    std::cout << "[DEV] Device Manager Terminated\n";
}


void DeviceManager::clear() {
    std::cout << "[DEV] clear: Cleanning...\n";

    if (thread_cam1.joinable()) {
        std::cout << "[DEV] Joining thread_rldp\n";
        thread_cam1.join();
    }
    delete cam1_stream;
    cam1_stream = nullptr;


    std::cout << "[DEV] clear: Cleanning Success\n";
}
