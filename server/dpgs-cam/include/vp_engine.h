#ifndef __VP_ENGINE_H__
#define __VP_ENGINE_H__

#include "cam_stream.h"
#include "str_frame_buffer.h"

#include <atomic>

// Internal buffer
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>


class VPEngine {
 public:
    VPEngine(CamStream& _cam_str, StrFrameBuffer& _fb);
    ~VPEngine();

    bool initialize();
    void run();
    void stop();

    bool is_run();


 private:
    std::atomic<bool>       is_running;

    cv::Mat image_processing(cv::Mat resized);
    void clear();


    // External Interface
    CamStream&      cam_str;
    StrFrameBuffer& fb;

};



#endif // __VP_ENGINE_H__
