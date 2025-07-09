#ifndef __CORE_PROCESS_SUPERVISOR_H__
#define __CORE_PROCESS_SUPERVISOR_H__

#include "frame_buffer.h"
#include "map_manager.h"

#include <sys/types.h>


class CoreProcessSupervisor {
 public:
    CoreProcessSupervisor(FrameBuffer& _fb, MapManager& _map_mgr);
    ~CoreProcessSupervisor();

    void start();
    void stop();

    bool monitor();

 private:
    pid_t   pid_ai = -1;
    bool    is_running_ai = false;

    FrameBuffer&    fb;
    MapManager&     map_mgr;

    bool create_process();
    bool clear();

};



#endif // __CORE_PROCESS_SUPERVISOR_H__
