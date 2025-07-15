#ifndef __VP_ENGINE_H__
#define __VP_ENGINE_H__

#include "cam_streaming_client.h"
#include "frame_buffer.h"
#include "frame_buffer_str.h"

#include <atomic>


class VPEngine {
 public:
    VPEngine(FrameBuffer& _fb);
    ~VPEngine();

    bool initialize();
    void run();
    void stop();

    bool is_run();

 private:
    std::atomic<bool>       is_running;

    CamStreamingClient*     csc;
    FrameBufferStr*         clt_fb1;
    FrameBufferStr*         clt_fb2;

    void clear();

    // External Interface
    FrameBuffer&    fb;

};



#endif // __VP_ENGINE_H__
