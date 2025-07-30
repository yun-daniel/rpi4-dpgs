#ifndef __SERVER_STREAMING_MODULE_H__
#define __SERVER_STREAMING_MODULE_H__

#include "str_frame_buffer.h"

#include <opencv2/opencv.hpp>

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <gio/gio.h>

#include <iostream>
#include <chrono>
#include <thread>
#include <ctime>


class SrvStream {
 public:
    SrvStream(StrFrameBuffer& _fb);
    ~SrvStream();

    bool initialize();
    void run();
    void stop();

 private:
    bool is_running = false;

    static void media_configure(GstRTSPMediaFactory* factory, GstRTSPMedia* media, gpointer user_data);

    void push_to_server(const cv::Mat& frame);
    void clear();


    // External Interface
    StrFrameBuffer& fb;

};


#endif // __SERVER_STREAMING_MODULE_H__
