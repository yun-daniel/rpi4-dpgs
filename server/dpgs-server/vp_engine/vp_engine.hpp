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

class VPEngine{
    public:
    void run();
    
    private:
    static queue<Mat> frame_queue;
    static mutex queue_mutex;
    static condition_variable queue_cv;
    
    Mat image_processing(Mat resized);
    void run_video_loop();
    void send_image(const Mat& processed);
    
    friend void recv_image();
    friend class ClientIF;
};

#endif