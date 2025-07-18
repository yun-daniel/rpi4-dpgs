#ifndef __VP_ENGINE_H__
#define __VP_ENGINE_H__

#include "cam_streaming_client.h"
#include "frame_buffer.h"
#include "frame_buffer_str.h"

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

    // Internal Buffer
    FrameBufferStr* clt_fb1;
    FrameBufferStr* clt_fb2;


 private:
    std::atomic<bool>       is_running;

    CamStreamingClient*     csc;

    void clear();


    // External Interface
    FrameBuffer&    fb;

};



#endif // __VP_ENGINE_H__
