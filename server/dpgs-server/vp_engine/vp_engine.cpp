#include "vp_engine.h"


VPEngine::VPEngine(FrameBuffer& _fb)
    : fb(_fb) {

}

VPEngine::~VPEngine() {
}


bool VPEngine::initialize() {
    std::cout << "[VPE] Start to initialize...\n";

    csc = new CamStreamingClient();
    if (!csc->initialize()) {
        std::cerr << "[VPE] Error: Failed to initialize Camera Streaming Client\n";
        return false;
    }

    std::cout << "[VPE] Success: Video Processing Engine initialized\n";
    return true;
}


void VPEngine::run() {
    std::cout << "[VPE] Start Video Processing Engine\n";

    cv::Mat frame, resized, processed;

    is_running = true;
    while (is_running) {

        if (!csc->frame_sampling(frame)) {
            std::cerr << "[VPE] sampling: Failed to read frame from input stream.\n";
            break;
        }

        cv::resize(frame, resized, cv::Size(640, 360), 0, 0, cv::INTER_AREA);

        cv::imshow("RTSP Raw Video (CCTV->RPI)", resized);

        fb.push(resized);

//        cv::imshow("RTSP Raw Video (CCTV->RPI)", resized);

    }

}


void VPEngine::stop() {
    is_running = false;
}
