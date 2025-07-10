#ifndef __VP_ENGINE_H__
#define __VP_ENGINE_H__

#include "frame_buffer.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>
#include <atomic>


class VPEngine {
 public:
    VPEngine(const std::string& _path, FrameBuffer& _fb);

    void run();
    void stop();

 private:
    std::string         path;
    FrameBuffer&        fb;

    std::atomic<bool>   is_running;

};



#endif // __VP_ENGINE_H__
