#ifndef __VP__ENGINE__HPP
#define __VP__ENGINE__HPP

#include <opencv2/opencv.hpp>

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/rtsp-server/rtsp-server.h>

#include <iostream>
#include <chrono>
#include <gio/gio.h>

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

using namespace std;
using namespace cv;
using namespace chrono;

#define MAX_QUEUE_SIZE 30

class VPEngine{
public:
    VPEngine();
    ~VPEngine();

    void run();
    
private:
    string cctv_input_streaming_pipeline;

    static queue<Mat> frame_queue_1, frame_queue_2;
    static mutex queue_mutex_1, queue_mutex_2;
    static condition_variable queue_cv_1, queue_cv_2;
    
    Mat image_processing(Mat resized_image);
    void start();
    void send_image(const Mat& processed);

    
    friend class StreamingModule;
};

#endif