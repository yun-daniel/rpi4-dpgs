#include "core_thread_manager.h"
#include "vp_engine_dummy.h"

#include <iostream>


CoreThreadManager::CoreThreadManager(FrameBuffer& _fb, MapManager& _map_mgr)
    : fb(_fb), map_mgr(_map_mgr) {
}

CoreThreadManager::~CoreThreadManager() {
}


void CoreThreadManager::start() {
    std::cout << "[THREAD_MGR] Start Thread Manager\n";

    thread_vpe = std::thread([this]() {
        VpeDummy vpe("tb/test_a.mp4", fb);
        vpe.start();
    });
        

}


void CoreThreadManager::stop() {
    std::cout << "[THREAD_MGR] Stop Thread Manager\n";

}


bool CoreThreadManager::create_thread(void *(*fptr)(void *arg)) {
    std::cout << "[THREAD_MGR] Create Thread...\n";



    std::cout << "[THREAD_MGR] Success: Thread created\n";
    return true;
}


bool CoreThreadManager::clear() {
    std::cout << "[THREAD_MGR] Cleanning...\n";


    std::cout << "[THREAD_MGR] Success: Thread clear\n";
    return true;
}


/*
int CoreThreadManager::add_thread(void *(*fptr)(void *arg)) {

    if (thread_cnt >= MAX_THREAD) {
        fprintf(stderr, "Error: Thread list is full\n");
        return 1;
    }
    
    if (pthread_create(&threads[thread_cnt], NULL, *fptr, NULL) != 0) {
        fprintf(stderr, "Error: Create thread for core threads[%d] failed", thread_cnt);
        return 1;
    }

    thread_cnt++;

    return 0;
}


int CoreThreadManager::clear() {

    int i;
    for (i = 0; i < thread_cnt; i++) {
        printf("Delete PID(Layer 2): %lx\n", threads[i]);

        if (pthread_cancel(threads[i]) != 0) {
            fprintf(stderr, "Error: %d's pthread_cancel failure\n", i);
            return 1;
        }
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Error: %d's pthread_join failure\n", i);
            return 1;
        }
        fprintf(stderr, "Terminate the threads[%d]\n", i);
    }

    return 0;
}
*/


