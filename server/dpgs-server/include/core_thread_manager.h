#ifndef CORE_THREAD_MANAGER_H
#define CORE_THREAD_MANAGER_H

#include <cstdio>

#include <pthread.h>

#define MAX_THREAD  5       /* Maximum number of core threads */

class CoreThreadManager
{
private:
    int thread_cnt;
    pthread_t threads[MAX_THREAD];

public:
    CoreThreadManager();
    ~CoreThreadManager();
    int add_thread(void *(*fptr)(void *arg));
    int clear();
};

#endif  // CORE_THREAD_MANAGER_H