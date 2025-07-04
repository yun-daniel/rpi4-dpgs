#ifndef __VPE_DUMMY_H__
#define __VPE_DUMMY_H__

#include "../frame_buffer.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>
#include <atomic>


class VpeDummy {
 public:
    VpeDummy(const std::string& _path, FrameBuffer& _fb);

    void start();

 private:
    void run();

    std::string path;
    FrameBuffer& fb; 

};



#endif // __VPE_DUMMY_H__
