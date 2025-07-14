#ifndef __VP_ENGINE_H__
#define __VP_ENGINE_H__

#include "frame_buffer.h"
#include "cam_streaming_client.h"

#include <atomic>


class VPEngine {
 public:
    VPEngine(FrameBuffer& _fb);
    ~VPEngine();

    bool initialize();
    void run();
    void stop();

 private:
    std::atomic<bool>       is_running;

    CamStreamingClient*     csc;


    // External Interface
    FrameBuffer&    fb;

};



#endif // __VP_ENGINE_H__
