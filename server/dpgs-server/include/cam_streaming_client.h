#ifndef __CAM_STREAMING_CLIENT_H__
#define __CAM_STREAMING_CLIENT_H__

#include "config.h"
#include <opencv2/opencv.hpp>


const std::string RTSP_SRC_PIPE = "rtspsrc location=rtsp://admin:Veda123%21@192.168.0.86:554/profile2/media.smp latency=30 ! queue ! rtph264depay ! queue ! h264parse ! queue ! avdec_h264 ! queue ! videoconvert ! queue ! appsink drop=true max-buffers=1";

// Only for Test
#if TEST_CAM_SRC
 #include <thread>
 #include <chrono>
 const int target_fps = 30;
 const int delay_ms = 1000 / target_fps;
#endif // TEST_CAM_SRC


class CamStreamingClient {
 public:
    bool initialize();

    bool frame_sampling(cv::Mat& frame);

 private:
    std::string stream_src;
    cv::VideoCapture cap;


    // Only for TEST
    bool is_test = false;

};




#endif // __CAM_STREAMING_CLIENT_H__
