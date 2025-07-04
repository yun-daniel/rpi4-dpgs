#include "core_thread_manager.h"

CoreThreadManager::CoreThreadManager() : thread_cnt(0) {
}

CoreThreadManager::~CoreThreadManager() {
}

/*
 * Creates a pthread that executes the function pointed to by fptr.
 * Returns 0 on success, 1 on failure.
 */
int CoreThreadManager::add_thread(void *(*fptr)(void *arg)) {

    if (thread_cnt >= MAX_THREAD) {
        fprintf(stderr, "Error: Thread list is full\n");
        return 1;
    }
    
    if (pthread_create(&threads[thread_cnt], NULL, *fptr, NULL) != 0) {
        fprintf(stderr, "Error: Create thread for threads[%d] failure", thread_cnt);
        return 1;
    }

    thread_cnt++;

    return 0;
}

/*
 * Terminate the threads[].
 * Returns 0 on success, 1 on failure.
 */
int CoreThreadManager::clear() {

    int i;
    for (i = 0; i < thread_cnt; i++) {
        printf("Delete PID: %lx\n", threads[i]);

        if (pthread_cancel(threads[i]) != 0) {
            fprintf(stderr, "%d's pthread_cancel failure\n", i);
            return 1;
        }
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "%d's pthread_join failure\n", i);
            return 1;
        }
        fprintf(stderr, "Terminate the threads[%d]\n", i);
    }

    return 0;
}



