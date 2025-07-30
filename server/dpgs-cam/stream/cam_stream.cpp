#include "cam_stream.h"


bool CamStream::initialize() {
    std::cout << "[CAM_STREAM] Start to initialize...\n";

    stream_src = "tb/test_a.mp4";
    cap.open(stream_src);

    if (!cap.isOpened()) {
        std::cerr << "[CAM_STREAM] Error: Failed to open input stream: " << stream_src << "\n";
        return false;
    }


    std::cout << "[CAM_STREAM] Success: Camera Streaming Module Initialized\n";
    return true;
}


bool CamStream::frame_sampling(cv::Mat& frame) {

    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    if (!cap.read(frame)) {
        std::cout << "[CAM_STREAM] sampling: Video Rewinding...\n";
        cap.set(cv::CAP_PROP_POS_FRAMES, 0);
        cap.read(frame);
    }

    if (!cap.read(frame)) {
        std::cerr << "[CAM_STREAM] sampling: Failed to read frame from source\n";
        return false;
    }

    return true;
}
