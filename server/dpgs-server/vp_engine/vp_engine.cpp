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

    clt_fb1 = new FrameBufferStr();
    if (!clt_fb1->initialize()) {
        std::cerr << "[VPE] Error: Failed to initialize Streaming Frame Buffer1\n";
        return false;
    }
    clt_fb2 = new FrameBufferStr();
    if (!clt_fb2->initialize()) {
        std::cerr << "[VPE] Error: Failed to initialize Streaming Frame Buffer2\n";
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

//        cv::imshow("RTSP Raw Video (CCTV->RPI)", resized);

        fb.push(resized);
        clt_fb1->push(resized);


    }

}


void VPEngine::stop() {
    std::cout << "[VPE] VPEngine Terminating...\n";

    is_running = false;

    clear();

    std::cout << "[VPE] VPEngine Terminated\n";
}

void VPEngine::clear() {
    std::cout << "[VPE] clear: Cleanning...\n";

    delete csc;
    csc = nullptr;

    delete clt_fb1;
    clt_fb1 = nullptr;

    delete clt_fb2;
    clt_fb2 = nullptr;

    std::cout << "[VPE] clear: Cleanning Success\n";
}


bool VPEngine::is_run() {
    return is_running;
}

