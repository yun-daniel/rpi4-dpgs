#ifndef __CAM_THREAD_SUPERVISOR_H__
#define __CAM_THREAD_SUPERVISOR_H__

//#include "str_frame_buffer.h"
//#include "cam_stream.h"
#include "vp_engine.h"
#include "srv_net.h"
//#include "ai_engine.h"

#include <sys/types.h>
#include <cstdio>
#include <thread>


class CamThrSupv {
 public:
    CamThrSupv(VPEngine& _vp_engine, SrvNet& _srv_net);
    ~CamThrSupv();

    bool initialize();
    void start();
    void stop();

    bool monitor();

 private:
    bool is_running = false;

    VPEngine&   vp_engine;
    SrvNet&     srv_net;
//    AIEngine*   ai_engine = nullptr;

    std::thread thread_vp_engine;
    std::thread thread_ai_engine;
    std::thread thread_srv_net;

    void clear();

};


#endif // __CAM_THREAD_SUPERVISOR_H__
