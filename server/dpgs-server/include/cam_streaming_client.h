#ifndef __CAM_STREAMING_CLIENT_H__
#define __CAM_STREAMING_CLIENT_H__

#include <opencv2/opencv.hpp>


const std::string RTSP_SRC_PIPE = "rtspsrc location=rtsp://admin:Veda123%21@192.168.0.86:554/profile2/media.smp latency=30 ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! appsink drop=true max-buffers=1";

// Only for Test
const std::string test_video = "tb/test_a.mp4";


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
