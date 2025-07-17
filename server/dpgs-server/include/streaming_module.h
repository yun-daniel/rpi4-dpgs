#ifndef __STREAMING_MODULE_H__
#define __STREAMING_MODULE_H__

#include "vp_engine.h"

#include <opencv2/opencv.hpp>

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/rtsp-server/rtsp-server.h>

#include <iostream>
#include <chrono>
#include <gio/gio.h>
#include <thread>

#include <ctime>

using namespace std;
using namespace cv;
using namespace chrono;


class StreamingModule{
public:
    StreamingModule(VPEngine& _vp_engine);
    ~StreamingModule();

    // static void* run(void* args);
    void update(int cam_id);
    // void clear();
    void run();

private:
    static void media_configure(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer user_data);
    void init_rtsps_server(const string& service_port, const string& path);
    void push_frame_to_rtsp(const Mat& frame);

    int cam_id_;
    pthread_mutex_t cam_mutex_;

    std::queue<cv::Mat>* selected_queue;
    std::mutex* selected_mutex;
    std::condition_variable* selected_cv;

    FrameBufferStr* selected_fb;


    // External Interface
//    FrameBufferStr& clt_fb1;
//    FrameBufferStr& clt_fb2;
    VPEngine& vp_engine;

};


#endif // __STREAMING_MODULE_H__
