#ifndef __CAM_RECEIVER_MODULE_H__
#define __CAM_RECEIVER_MODULE_H__

#include "str_frame_buffer.h"

#include <opencv2/opencv.hpp>

#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <gio/gio.h>


class CamRxModule {
 public:
    CamRxModule(const std::string& stream_src, StrFrameBuffer& _fb);
    ~CamRxModule();

    bool initialize();
    void run();
    void stop();

 private:
    bool is_running = false;

    std::string cam_ip = "10.0.2.15";
    int port = 9999;
    int sock;
    sockaddr_in cam_addr;

    std::string stream_src;
    cv::VideoCapture cap;


    // External Interface
    StrFrameBuffer& fb;

};


#endif // __CAM_RECEIVER_MODULE_H__
