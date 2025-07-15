#include "cam_streaming_client.h"

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/rtsp-server/rtsp-server.h>

// Only for Test
#include <thread>
#include <chrono>
const int target_fps = 30;
const int delay_ms = 1000 / target_fps;


bool CamStreamingClient::initialize() {

    is_test = CAM_SRC_TEST;

    gst_init(nullptr, nullptr);
    std::cout << "[CSC] GStreamer init\n";

    if (is_test) {
        stream_src = test_video;
        cap.open(stream_src);
    }
    else {
        stream_src = RTSP_SRC_PIPE;
        std::cout << "[CSC] test0\n";
        cap.open(stream_src, cv::CAP_GSTREAMER);
        std::cout << "[CSC] test2\n";
    }

    std::cout << "[CSC] test1\n";

    if (!cap.isOpened()) {
        std::cerr << "[CSC] initialize: Failed to open input stream: " << stream_src << "\n";
        return false;
    }


    return true;
}


bool CamStreamingClient::frame_sampling(cv::Mat& frame) {

    if (is_test) {
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }

    if (!cap.read(frame)) {
        std::cerr << "[CSC] sampling: Faile to read frame from stream\n";
        return false;
    }

    return true;
}
