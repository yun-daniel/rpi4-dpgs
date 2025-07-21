#include "cam_streaming_client.h"

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/rtsp-server/rtsp-server.h>


bool CamStreamingClient::initialize() {

    gst_init(nullptr, nullptr);
    std::cout << "[CSC] GStreamer init\n";

 #if TEST_CAM_SRC
    stream_src = TEST_VIDEO;
    cap.open(stream_src);
 #else
    stream_src = RTSP_SRC_PIPE;
    cap.open(stream_src, cv::CAP_GSTREAMER);
 #endif // TEST_CAM_SRC

    if (!cap.isOpened()) {
        std::cerr << "[CSC] initialize: Failed to open input stream: " << stream_src << "\n";
        return false;
    }

    return true;
}

bool CamStreamingClient::frame_sampling(cv::Mat& frame) {

 #if TEST_CAM_SRC
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    if (!cap.read(frame)) {
        std::cout << "[CSC] sampling: Video Rewinding...\n";
	cap.set(cv::CAP_PROP_POS_FRAMES, 0);
        cap.read(frame);
    }
 #endif // TEST_CAM_SRC

    if (!cap.read(frame)) {
        std::cerr << "[CSC] sampling: Failed to read frame from stream\n";
        return false;
    }

    return true;
}
