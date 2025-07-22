#ifndef __AI_ENGINE_H__
#define __AI_ENGINE_H__

#include <opencv2/opencv.hpp>

#include "frame_buffer.h"
#include "map_manager.h"


class AIEngine {
 public:
    AIEngine(FrameBuffer& _fb, MapManager& _mgr);
    ~AIEngine();

    bool initialize();
    void run();
    void stop();

 private:
    class Impl;
    Impl* impl;
};



#endif // __AI_ENGINE_H__
