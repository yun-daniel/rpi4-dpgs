#ifndef __SYSTEM_CONTROLLER_H__
#define __SYSTEM_CONTROLLER_H__

#include "frame_buffer.h"
#include "map_manager.h"
#include "core_thread_manager.h"
#include "core_process_supervisor.h"

#include <cstdio>
#include <pthread.h>

#define MAX_TASK    5


class SystemController {
 public:
    SystemController();
    ~SystemController();

    bool initialize();
    void start();
    void stop();

 private:
    std::unique_ptr<FrameBuffer>    fb;
    std::unique_ptr<MapManager>     map_mgr;

    std::unique_ptr<CoreThreadManager>      thread_mgr;
    std::unique_ptr<CoreProcessSupervisor>  proc_supv;

};



#endif // __SYSTEM_CONTROLLER_H__
