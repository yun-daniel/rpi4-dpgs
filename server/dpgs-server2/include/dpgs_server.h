#ifndef __DPGS_SERVER_H__
#define __DPGS_SERVER_H__

#include "str_frame_buffer.h"
#include "device_manager.h"
#include "map_manager.h"
#include "server_thread_supv.h"
//#include "config.h"
//#include "frame_buffer.h"
//#include "core_thr_supv.h"
//#include "core_proc_supv.h"

#include <cstdio>
#include <pthread.h>


class DPGSServer {
 public:
    DPGSServer();
    ~DPGSServer();

    bool initialize();
    void start();
    void stop();
    void clear();

 private:
    std::unique_ptr<StrFrameBuffer> fb;
    std::unique_ptr<DeviceManager>  dev_mgr;
    std::unique_ptr<SrvThrSupv>     thr_supv;
    std::unique_ptr<MapManager>     map_mgr;
//    std::unique_ptr<CoreProcSupv>   proc_supv;


};



#endif // __SYSTEM_CONTROLLER_H__
