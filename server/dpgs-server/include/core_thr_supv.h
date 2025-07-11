#ifndef __CORE_THREAD_SUPERVISOR_H__
#define __CORE_THREAD_SUPERVISOR_H__

#include "frame_buffer.h"
#include "map_manager.h"
#include "vp_engine_dummy.h"
#include "device_manager.h"

#include <sys/types.h>
#include <cstdio>
#include <thread>


class CoreThrSupv {
 public:
    CoreThrSupv(FrameBuffer& _fb, MapManager& _map_mgr);
    ~CoreThrSupv();

    bool initialize();
    void start();
    void stop();

private:
    VPEngine*       vp_engine = nullptr;
    DeviceManager*  dev_mgr = nullptr;

    std::thread     thread_vpe;
    std::thread     thread_dev;


    void clear();


    // External Interface
    FrameBuffer&    fb;
    MapManager&     map_mgr;

};

#endif  // __CORE_THREAD_SUPERVISOR_H__
