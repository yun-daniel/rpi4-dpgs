#include <iostream>
#include <cstdlib>
#include <pthread.h>

#include <unistd.h>
#include <vector>
#include <algorithm>

using namespace std;

#define THREAD_CNT  7               /* Total number of threads */
                                    /* (0th index is intentionally unused for clear numbering) */

pthread_t threads[THREAD_CNT];      // Core task handler threads 
vector<pthread_t> tid_mapdata;      // Threads for sending map data to each client
vector<pthread_t> tid_auth;         // Threads for handling ID/password authentication with clients

pthread_mutex_t m_mapdata = PTHREAD_MUTEX_INITIALIZER;      // Mutex for tid_mapdata
pthread_mutex_t m_auth = PTHREAD_MUTEX_INITIALIZER;         // Mutex for tid_auth

pthread_attr_t attr;                // Thread attributes for detach state

/*
Cleanup handler of pthread_routine1 and pthread_routine2
Request cancel to threads depends on arg
1 : tid_mapdata
2 : tid_auth
*/
void cleanup_handler1 (void *arg) {
    int option = *((int *)arg);
    vector<pthread_t> *v_ptr;
    pthread_mutex_t *m_ptr;
    free(arg);

    if (option == 1) {
        v_ptr = &tid_mapdata;
        m_ptr = &m_mapdata;
    }
    else if (option == 2) {
        v_ptr = &tid_auth;
        m_ptr = &m_auth;
    }
    else {
        return;
    }

    int i;
    pthread_mutex_lock(m_ptr);
        for (i = 0; i < v_ptr->size(); i++) {
            if (pthread_cancel((*v_ptr)[i]) != 0) {
                fprintf(stderr, "%d's pthread_cancel failure\n", i);
            }
    }
    fprintf(stderr, "%d handler1 : %lx\n", option, pthread_self());
    pthread_mutex_unlock(m_ptr);
}

/* 
Cleanup handler of subpthread of pthread_routine 1
Erase the element the same value of *arg
*/
void cleanup_handler2_mapdata (void *arg) {
    pthread_t tid = *((pthread_t *)arg);
    free (arg);

    pthread_mutex_lock(&m_mapdata);
        auto it = find(tid_mapdata.begin(), tid_mapdata.end(), tid);
        if (it == tid_mapdata.end()) {
            fprintf(stderr, "Find(cleanup_handler2_mapdata) failure\n");
            pthread_mutex_unlock(&m_mapdata);
            return;
        }
        fprintf(stderr, "Erase(mapdata) : %lx\n", tid);
        tid_mapdata.erase(it);
    pthread_mutex_unlock(&m_mapdata);
}

/*
Cleanup handler of subpthread of pthread_routine2
Erase the element that same value of *arg
*/
void cleanup_handler2_auth (void *arg) {
    pthread_t tid = *((pthread_t *)arg);
    free(arg);

    pthread_mutex_lock(&m_auth);
        auto it = find(tid_auth.begin(), tid_auth.end(), tid);
        if (it == tid_auth.end()) {
            fprintf(stderr, "Find(cleanup_handler2_auth) failure\n");
            pthread_mutex_unlock(&m_auth);
            return;
        }   
        fprintf(stderr, "Erase(auth) : %lx\n", tid);
        tid_auth.erase(it);
    pthread_mutex_unlock(&m_auth);
}

/*
thread for send map data to client socket
arg contains client socket fd
*/
void * send_mapdata (void *arg) {
    int clnt_sock = *((int *)arg);
    free(arg);

    pthread_t *tid_ptr = (pthread_t *)malloc(sizeof(pthread_t));
    *tid_ptr = pthread_self();
    pthread_cleanup_push(cleanup_handler2_mapdata, tid_ptr);

    // send map data to clnt_sock
    
    printf("threads[send_mapdata] : %lx\n", pthread_self());
    while (1) {
        sleep(1);
    }

    pthread_cleanup_pop(1);     // It will be executed when the client is closed(after pthread_exit)

    return nullptr;
}

/*
thread for ID/password authentication with clients
arg contains client socket fd
*/
void * auth (void *arg) {
    int clnt_sock = *((int *)arg);
    free(arg);
 
    pthread_t *tid_ptr = (pthread_t *)malloc(sizeof(pthread_t));
    *tid_ptr = pthread_self();
    pthread_cleanup_push(cleanup_handler2_auth, tid_ptr);

    // send map data to clnt_sock
    
    printf("threads[auth] : %lx\n", pthread_self());
    while (1) {
        sleep(1);
    }

    pthread_cleanup_pop(1);     // It will be excuted when the client is closed(after pthread_exit)   

    return nullptr;
}
                                   

void * pthread_routine1 (void *arg) {
    printf("threads[1] : %lx\n", pthread_self());
    
    int *option_ptr = (int *)malloc(sizeof(int));
    *option_ptr = 1;
    pthread_cleanup_push(cleanup_handler1, option_ptr);

    // create listen_fd(bind, listen)

    // accept socket
    int *clnt_sock;
    pthread_t tid;
    while (1) {
        pthread_testcancel();
        clnt_sock = (int *)malloc(sizeof(int));
        // *clnt_sock = accept(listen_fd,...);

        // add tid
        if (pthread_create(&tid, &attr, send_mapdata, (void *)clnt_sock) != 0) {
            fprintf(stderr, "%d's pthread_create of mapdata is failed\n", *clnt_sock);
            free (clnt_sock);
        } else {
            pthread_mutex_lock(&m_mapdata);
                tid_mapdata.push_back(tid);
            pthread_mutex_unlock(&m_mapdata);
        }

        sleep(3);
    }

    pthread_cleanup_pop(1);     // This will be not excuted

    return nullptr;
}

void * pthread_routine2 (void *arg) {
    printf("threads[2] : %lx\n", pthread_self());

    int *option_ptr = (int *)malloc(sizeof(int));
    *option_ptr = 2;
    pthread_cleanup_push(cleanup_handler1, option_ptr);
    
    // create listen_fd

    // accept socket
    int *clnt_sock;
    pthread_t tid;
    while (1) {
        pthread_testcancel();
        clnt_sock = (int *)malloc(sizeof(int));
        // *clnt_sock = accept(listen_fd, ..);

        // add tid
        if (pthread_create(&tid, &attr, auth, (void *)clnt_sock) != 0) {
            fprintf(stderr, "%d's pthread_create of auth is failed\n", *clnt_sock);
            free (clnt_sock);
        } else {
            pthread_mutex_lock(&m_auth);
                tid_auth.push_back(tid);
            pthread_mutex_unlock(&m_auth);
        }

        sleep(3);
    }

    pthread_cleanup_pop(1);     // This will be not excuted

    return nullptr;
}

void * pthread_routine3 (void *arg) {
    printf("threads[3] : %lx\n", pthread_self());
    int i = 0;
    while (1) {
        printf("threads3 : %d\n", i++);
        sleep(1);
    }
    return nullptr;
}

void * pthread_routine4 (void *arg) {
    printf("threads[4] : %lx\n", pthread_self());
    int i = 0;
    while (1) {
        printf("threads4 : %d\n", i++);
        sleep(1);
    }
    return nullptr;
}

void * pthread_routine5 (void *arg) {
    printf("threads[5] : %lx\n", pthread_self());
    int i = 0;
    while (1) {
        printf("threads5: %d\n", i++);
        sleep(1);
    }
    return nullptr;
}

void * pthread_routine6 (void *arg) {
    printf("threads[6] : %lx\n", pthread_self());
    int i = 0;
    while (1) {
        printf("threads6 : %d\n", i++);
        sleep(1);
    }
    return nullptr;
}

/* 
Create threads for each task

Thread 1 : client connection manager (send map data)
Thread 2 : client connection manager (receive id/pw)
Thread 3 : camera interface
Thread 4 : ISP
Thread 5 : device driver (sensor)
Thread 6 : device driver (actuator)

if excute successfully return 0, otherwise return 1
*/
int setting_threads (void) {

    // setting detach option of pthread
    if (pthread_attr_init(&attr) != 0) {
        fprintf(stderr, "pthread_attr_init failure\n");
        return 1;
    }
    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
        fprintf(stderr, "pthread_attr_setdetachstate failure\n");
        return 1; 
    }

    // Thread 1 : client connection manager (send map data)
    if (pthread_create(&threads[1], NULL, pthread_routine1, NULL) != 0) {
        fprintf(stderr, "thread1 creation fail\n");
        return 1;
    }

    // Thread 2 : client connection manager (receive id/pw)
    if (pthread_create(&threads[2], NULL, pthread_routine2, NULL) != 0) {
        fprintf(stderr, "thread2 creation fail\n");
        return 1;
    }
    
    // Thread 3 : camera interface
    if (pthread_create(&threads[3], NULL, pthread_routine3, NULL) != 0) {
        fprintf(stderr, "thread3 creation fail\n");
        return 1;
    }

    // Thread 4 : ISP
    if (pthread_create(&threads[4], NULL, pthread_routine4, NULL) != 0) {
        fprintf(stderr, "thread4 creation fail\n");
        return 1;
    }

    // Thread 5 : device driver (sensor)
    if (pthread_create(&threads[5], NULL, pthread_routine5, NULL) != 0) {
        fprintf(stderr, "thread5 creation fail\n");
        return 1;
    }

    // Thread 6 : device driver (actuator)
    if (pthread_create(&threads[6], NULL, pthread_routine6, NULL) != 0) {
        fprintf(stderr, "thread6 creation fail\n");
        return 1;
    }
        

    return 0;
}

// Performs tasks for a safe exit
void exit_routine() {
    int i;
    // Terminate the threads[]
    for (i = 1; i < THREAD_CNT; i++) {
        if (pthread_cancel(threads[i]) != 0) {
            fprintf(stderr, "%d's pthread_cancel failure\n", i);
        }
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "%d's pthread_join failure\n", i);
        }
        fprintf(stderr, "Terminate the threads[%d]\n", i);
    } 
    
    sleep(5);       // for wating cleanup handler

    pthread_attr_destroy(&attr);
}

int main (void) {

    if (setting_threads() == 1) {
        fprintf(stderr, "setting_threads() failed\n");
        exit(EXIT_FAILURE);
    }

    // Multi process : AI model & prediction algorithm


    int input;
    printf(">> ");
    scanf("%d", &input);

    if (input < 0) {
        exit_routine();
    }

    return 0;

}