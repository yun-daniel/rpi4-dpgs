#ifndef __DPGS_CAM_H__
#define __DPGS_CAM_H__

#include "cam_stream.h"
#include "str_frame_buffer.h"
#include "vp_engine.h"
#include "srv_net.h"
#include "cam_thr_supv.h"


class DPGSCam {
 public:
    DPGSCam();
    ~DPGSCam();

    bool initialize();
    void start();
    void stop();

 private:
    std::unique_ptr<CamStream>      cam_stream;
    std::unique_ptr<StrFrameBuffer> fb;
    std::unique_ptr<VPEngine>       vp_engine;
    std::unique_ptr<SrvNet>         srv_net;
//    std::unique_ptr<SrvStream>      srv_stream;
//    std::unique_ptr<AIEngine>       ai_engine;
    std::unique_ptr<CamThrSupv>     cam_thr_supv;

    void clear();

};


#endif // __DPGS_CAM_H__
