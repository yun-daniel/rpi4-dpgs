#include "dpgs_cam.h"

#include <iostream>
#include <thread>
#include <chrono>


DPGSCam::DPGSCam() {
}

DPGSCam::~DPGSCam() {
}


bool DPGSCam::initialize() {
    std::cout << "[CAM] Start to initialize...\n";

    std::cout << "[CAM] Initialize Camera Streaming Module\n";
    cam_stream = std::make_unique<CamStream>();
    if (!cam_stream->initialize()) {
        std::cerr << "[CAM] Error: Failed to initialize CamStream\n";
        return false;
    }

    std::cout << "[CAM] Initialize Streaming Frame Buffer\n";
    fb = std::make_unique<StrFrameBuffer>();
    if (!fb->initialize()) {
        std::cerr << "[CAM] Error: Failed to initialize StrFrameBuffer\n";
        return false;
    }

    std::cout << "[CAM] Initialize Video Processing Engine\n";
    vp_engine = std::make_unique<VPEngine>(*cam_stream, *fb);
    if (!vp_engine->initialize()) {
        std::cerr << "[CAM] Error: Failed to initialize VPEngine\n";
        return false;
    }

    std::cout << "[CAM] Initialize Server Network Module\n";
    srv_net = std::make_unique<SrvNet>(*fb);
    if (!srv_net->initialize()) {
        std::cerr << "[CAM] Error: Failed to initialize SrvNet\n";
        return false;
    }

    std::cout << "[CAM] Initialize Thread Supervisor...\n";
    cam_thr_supv = std::make_unique<CamThrSupv>(*vp_engine, *srv_net);
    if (!cam_thr_supv->initialize()) {
        std::cerr << "[CAM] Error: Failed to initialize CamThrSupv\n";
        return false;
    }


    std::cout << "[CAM] Success: Camera SW Initialized\n";
    return true;
}

void DPGSCam::start() {
    std::cout << "[CAM] Camera SW Start\n";

    cam_thr_supv->start();

    while (cam_thr_supv->monitor()) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

}


void DPGSCam::stop() {
    std::cout << "[CAM] Camera SW Terminating...\n";

    cam_thr_supv->stop();

    clear();

    std::cout << "[CAM] Camera SW Terminated\n";
}


void DPGSCam::clear() {
    std::cout << "[CAM] clear: Cleanning..\n";

    cam_thr_supv.reset();
    srv_net.reset();
    vp_engine.reset();
    fb.reset();
    cam_stream.reset();

    std::cout << "[CAM] clear: Cleanning Success\n";
}

