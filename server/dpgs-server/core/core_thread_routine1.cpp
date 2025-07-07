#include "core_thread_routine.h"
#include "client_thread_manager.h"

/*
 * Cleanup handler for core_thread_routine1.
 * Cancels and joins the two subthreads (threads[2]).
 */
void core_thread1_cleanup_handler (void *arg) {
    pthread_t *threads = (pthread_t *)arg;

    int i;
    for (i = 1; i >= 0; i--) {
        printf("Delete PID(Layer 3): %lx\n", threads[i]);

        if (pthread_cancel(threads[i]) != 0) {
            fprintf(stderr, "Error: %d's pthread_cancel failure\n", i);
        }
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Error: %d's pthread_join failure\n", i);
        }
        fprintf(stderr, "Terminate the threads[%d]\n", i);
    }
}

void run_client_thread_manager_cleanup_handler (void *arg) {
    ClientThreadManager *ctm_ptr = (ClientThreadManager *)arg;

    ctm_ptr->clear():
}

/*
 * check map changed_flag + signal
 */
void * check_map_manager (void *arg) {
    while(1) {
        sleep(1);
    }
    return nullptr;
}

/*
 * initalize the client_thread_manager and run
 * connecting with multi clients
 */
void * run_client_thread_manager (void *arg) {
    pthread_cleanup_push(run_client_thread_manager_cleanup_handler, arg);

    ClientThreadManager *ctm_ptr = (ClientThreadManager *)arg;
    ctm_ptr->initalize();
    ctm_ptr->connect_client();
    
    return nullptr;
}


void * core_thread_routine1 (void *arg) {
    pthread_t threads[2];
    ClientThreadManager ctm;

    pthread_cleanup_push(core_thread1_cleanup_handler, threads);

    if (pthread_create(&threads[0], NULL, check_map_manager, (void *)&ctm) != 0) {
        fprintf(stderr, "Error: Create thread for core_thread_routine1's threads[0] failed\n");
        pthread_exit(NULL);
    }

    if (pthread_create(&threads[1], NULL, run_client_thread_manager, (void *)&ctm) != 0) {
        fprintf(stderr, "Error: Create thread for core_thread_routine1's threads[1] failed\n");
        pthread_exit(NULL);
    }

    // NOTE:
    // The following join calls are unlikely to be reached during normal execution,
    // because both subthreads (threads[0] and threads[1])
    // are designed to run infinite loops (e.g., while(1)).
    //
    // Therefore, this routine (core_thread_routine1) will typically be cancelled 
    // from outside, triggering the registered cleanup handler (core_thread1_cleanup_handler)
    // instead of reaching the pthread_join() and pthread_cleanup_pop() below.
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    pthread_cleanup_pop(1);

    return nullptr;
}



