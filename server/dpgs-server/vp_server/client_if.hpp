#ifndef __CLIENT__IF__HPP
#define __CLIENT__IF__HPP

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

class ClientIF{
public:
    static void* run(void* args);

private:

    static void media_configure(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer user_data);
    static void init_rtsp_server(const string& service_port, const string& path);
    static void push_frame_to_rtsp(const Mat& frame);
    static void recv_image(int queue_index);
    Mat create_dummy_frame(int queue_index);
};

#endif