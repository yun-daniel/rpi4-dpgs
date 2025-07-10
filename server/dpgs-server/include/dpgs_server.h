#ifndef __DPGS_SERVER_H__
#define __DPGS_SERVER_H__

#include "frame_buffer.h"
#include "map_manager.h"
#include "core_thr_supv.h"
#include "core_proc_supv.h"

#include <cstdio>
#include <pthread.h>


class DPGSServer {
 public:
    DPGSServer();
    ~DPGSServer();

    bool initialize();
    void start();
    void stop();

 private:
    std::unique_ptr<FrameBuffer>    fb;
    std::unique_ptr<MapManager>     map_mgr;
    std::unique_ptr<CoreThrSupv>    thr_supv;
    std::unique_ptr<CoreProcSupv>   proc_supv;

    bool initialize_proc_supv();
    bool initialize_thr_supv();
    void clear();

};



#endif // __SYSTEM_CONTROLLER_H__
