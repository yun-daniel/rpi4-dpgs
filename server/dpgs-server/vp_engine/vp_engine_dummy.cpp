#include "vp_engine_dummy.h"
#include <chrono>
#include <iostream>


const int target_fps = 30;
const int delay_ms = 1000 / target_fps;


VPEngine::VPEngine(const std::string& _path, FrameBuffer& _fb)
    : path(_path), fb(_fb) {
}


void VPEngine::run() {
    cv::VideoCapture test_video(path);
    if (!test_video.isOpened()) {
        std::cerr << "[VPED] Error: Failed to open video: " << path << "\n";
        return;
    }

    std::cout << "[VPED] Success: test_video is opened and start to push\n";

    cv::Mat frame;

    is_running = true;
    while (is_running) {
        if (!test_video.read(frame)) {
            test_video.set(cv::CAP_PROP_POS_FRAMES, 0);
            continue;
        }

        // Debug Session
//        size_t size = frame.total() * frame.elemSize();
//        std::cout << "[DEBUG][VPED] push: Frame Size: " << size << ", total: " << frame.total() << ", rows: " << frame.rows << ", cols: " << frame.cols <<  "\n";
        // ------

        fb.push(frame);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }

    test_video.release();
}


void VPEngine::stop() {
    is_running = false;
}

