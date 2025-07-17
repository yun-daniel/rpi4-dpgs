#ifndef __VP_ENGINE_H__
#define __VP_ENGINE_H__

#include "cam_streaming_client.h"
#include "frame_buffer.h"
//#include "frame_buffer_str.h"

#include <atomic>

// Internal buffer
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>


class VPEngine {
 public:
    VPEngine(FrameBuffer& _fb);
    ~VPEngine();

    bool initialize();
    void run();
    void stop();

    bool is_run();

//    FrameBufferStr* get_clt_fb(int idx);
    // Internal Buffer
    std::queue<cv::Mat> frame_queue_1, frame_queue_2;
    std::mutex	queue_mutex_1, queue_mutex_2;
    std::condition_variable queue_cv_1, queue_cv_2;


 private:
    std::atomic<bool>       is_running;

    CamStreamingClient*     csc;
//    FrameBufferStr*         clt_fb1;
//    FrameBufferStr*         clt_fb2;

    void clear();


    // External Interface
    FrameBuffer&    fb;

};



#endif // __VP_ENGINE_H__
