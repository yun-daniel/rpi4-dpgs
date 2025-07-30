#ifndef __DEVICE_MANAGER_H__
#define __DEVICE_MANAGER_H__

#include "str_frame_buffer.h"
#include "cam_rx_module.h"

#include <sys/types.h>
#include <thread>

const std::string CAM1_SRC_PIPE = "rtspsrc location=rtsp://10.0.2.15:8555/stream latency=30 ! queue ! rtph264depay ! queue ! h264parse ! queue ! avdec_h264 ! queue ! videoconvert ! queue ! appsink drop=true max-buffers=1";


class DeviceManager {
 public:
    DeviceManager(StrFrameBuffer& _fb);
    ~DeviceManager();

    bool initialize();
    void run();
    void stop();

 private:
    bool is_running = false;
    bool cam1_en = false;
    bool cam2_en = false;

    CamRxModule*    cam1_stream;

    std::thread thread_cam1;
    std::thread thread_cam2;


    void clear();


    // External Interface
    StrFrameBuffer&     fb;

};



#endif // __DEVICE_MANAGER_H__
