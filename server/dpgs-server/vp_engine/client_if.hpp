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

using namespace std;
using namespace cv;
using namespace chrono;

class ClientIF{
public:
    void run();

private:
    static void media_configure(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer user_data);
    void init_rtsp_server(const string& service_port, const string& path);
    void push_frame_to_rtsp(const Mat& frame);
    void recv_image();

};

#endif