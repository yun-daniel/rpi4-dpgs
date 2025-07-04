#include "core_thread_routine.h"

typedef struct {
    pthread_t *threads;
    // ClientThreadManager obj;
    // mapdata 
} CoreThread1Data;

/*
 * Cleanup handler for core_thread_routine1.
 * It performs the following cleanup tasks:
 *  1. Cancels and joins the two subthreads (ctd->threads).
 *  2. Frees the dynamically allocated thread array (ctd->threads).
 *  3. Frees the CoreThread1Data structure itself (ctd).
 */
void core_thread1_cleanup_handler (void *arg) {
    CoreThread1Data * ctd = (CoreThread1Data *)arg;

    int i;
    for (i = 0; i < 2; i++) {
        printf("Delete PID(Layer 3): %lx\n", ctd->threads[i]);

        if (pthread_cancel(ctd->threads[i]) != 0) {
            fprintf(stderr, "Error: %d's pthread_cancel failure\n", i);
        }
        if (pthread_join(ctd->threads[i], NULL) != 0) {
            fprintf(stderr, "Error: %d's pthread_join failure\n", i);
        }
        fprintf(stderr, "Terminate the threads[%d]\n", i);
    }

    free (ctd->threads);
    // TODO : have to know when the ctd->obj's clear is done
    free (ctd);

    fprintf(stderr, "Free is done\n");
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
    while(1) {
        sleep(1);
    }
    return nullptr;
}


void * core_thread_routine1 (void *arg) {
    // Dynamic allocation for data of core_thread_routine1
    CoreThread1Data * ctd;
    if ((ctd = (CoreThread1Data *)malloc(sizeof(CoreThread1Data))) == NULL) {
        perror("Error: Malloc ctd failed\n");
        pthread_exit(NULL);
    }
    if ((ctd->threads = (pthread_t *)malloc(sizeof(pthread_t) * 2)) == NULL) {
        perror("Error: Malloc ctd->threads failed\n");
        pthread_exit(NULL);
    }

    pthread_cleanup_push(core_thread1_cleanup_handler, ctd);

    if (pthread_create(&(ctd->threads[0]), NULL, check_map_manager, (void *)ctd) != 0) {
        fprintf(stderr, "Error: Create thread for core_thread_routine1's threads[0] failed\n");
        pthread_exit(NULL);
    }

    if (pthread_create(&(ctd->threads[1]), NULL, run_client_thread_manager, (void *)ctd) != 0) {
        fprintf(stderr, "Error: Create thread for core_thread_routine1's threads[1] failed\n");
        pthread_exit(NULL);
    }

    // NOTE:
    // The following join calls are unlikely to be reached during normal execution,
    // because both subthreads (ctd->threads[0] and ctd->threads[1])
    // are designed to run infinite loops (e.g., while(1)).
    //
    // Therefore, this routine (core_thread_routine1) will typically be cancelled 
    // from outside, triggering the registered cleanup handler (core_thread1_cleanup_handler)
    // instead of reaching the pthread_join() and pthread_cleanup_pop() below.
    pthread_join(ctd->threads[0], NULL);
    pthread_join(ctd->threads[1], NULL);

    pthread_cleanup_pop(1);

    return nullptr;
}



