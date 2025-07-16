#ifndef __STREAMING__MODULE__HPP
#define __STREAMING__MODULE__HPP

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
    StreamingModule();
    ~StreamingModule();

    static void* run(void* args);

private:
    static void media_configure(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer user_data);
    void init_rtsps_server(const string& service_port, const string& path);
    void push_frame_to_rtsp(const Mat& frame);
    void start_streaming(int queue_index);
};

#endif