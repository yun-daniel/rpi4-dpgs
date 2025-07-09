#ifndef __CORE_THREAD_MANAGER_H__
#define __CORE_THREAD_MANAGER_H__

#include "frame_buffer.h"
#include "map_manager.h"

#include <sys/types.h>
#include <cstdio>
#include <pthread.h>
#include <thread>

#define MAX_THREAD  5


class CoreThreadManager {
 public:
    CoreThreadManager(FrameBuffer& _fb, MapManager& _map_mgr);
    ~CoreThreadManager();

    void start();
    void stop();

private:
    int thread_cnt;
    pthread_t threads[MAX_THREAD];

    FrameBuffer&    fb;
    MapManager&     map_mgr;

    std::thread     thread_vpe;

    bool create_thread(void *(*fptr)(void *arg));
    bool clear();

};

#endif  // __CORE_THREAD_MANAGER_H__
