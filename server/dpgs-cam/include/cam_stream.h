#ifndef __CAM_STREAMING_MODULE_H__
#define __CAM_STREAMING_MODULE_H__

#include <opencv2/opencv.hpp>


#include <thread>
#include <chrono>
const int target_fps = 30;
const int delay_ms = 1000 / target_fps;


class CamStream {
 public:
    bool initialize();

    bool frame_sampling(cv::Mat& frame);

 private:
    std::string stream_src;
    cv::VideoCapture cap;

};


#endif // __CAM_STREAMING_MODULE_H__
