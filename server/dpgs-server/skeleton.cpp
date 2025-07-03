#include <iostream>
#include <cstdlib>
#include <vector>
#include <algorithm>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>

using namespace std;

#define THREAD_CNT  7               /* Total number of threads */
                                    /* (0th index is intentionally unused for clear numbering) */

pid_t ai_pid;                       // Process to run AI model

pthread_t threads[THREAD_CNT];      // Core task handler threads 
vector<pthread_t> tid_mapdata;      // Threads for sending map data to each client
vector<pthread_t> tid_auth;         // Threads for handling ID/password authentication with clients

pthread_mutex_t m_mapdata = PTHREAD_MUTEX_INITIALIZER;      // Mutex for tid_mapdata
pthread_mutex_t m_auth = PTHREAD_MUTEX_INITIALIZER;         // Mutex for tid_auth

/*
 * cleanup_handler1: cleanup handler for pthread_routine1 and pthread_routine2
 * Cancels threads based on the arg
 *   1: cancel all threads in tid_mapdata
 *   2: cancel all threads in tid_auth
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

    vector<pthread_t> v_copy;
    pthread_mutex_lock(m_ptr);
        v_copy = *v_ptr;
        for (pthread_t tid : *v_ptr) {
            if (pthread_cancel(tid) != 0) {
                fprintf(stderr, "%lx pthread_cancel failure\n", tid);
            }
        }
    fprintf(stderr, "%d handler1 : %lx\n", option, pthread_self());
    pthread_mutex_unlock(m_ptr);

    int ret;
    for (pthread_t tid : v_copy) {
        ret = pthread_join(tid, NULL);
        if (ret == ESRCH) {
            fprintf(stderr, "%lx is already deleted\n", tid);
        }
    }
}

/*
 * cleanup_handler2_mapdata:
 * Cleanup handler for sub-threads created by pthread_routine1
 * Deletes from tid_mapdata the thread based on the arg
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
 * cleanup_handler2_auth:
 * Cleanup handler for sub-threads created by pthread_routine2
 * Deletes from tid_auth the thread based on the arg
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
 * Thread that sends map data to a client
 * arg points to the client socket file descriptor
 */
void * send_mapdata (void *arg) {
    int clnt_sock = *((int *)arg);
    free(arg);

    pthread_t *tid_ptr = (pthread_t *)malloc(sizeof(pthread_t));
    *tid_ptr = pthread_self();
    pthread_cleanup_push(cleanup_handler2_mapdata, tid_ptr);

    // send map data to clnt_sock
    
    printf("PID[send_mapdata] : %lx\n", pthread_self());
    while (1) {
        sleep(1);
    }

    pthread_cleanup_pop(1);     // It will be executed when the client is closed(after pthread_exit)

    return nullptr;
}

/*
 * Thread that handles ID/password authentication with a client
 * arg points to the client socket file descriptor
 */
void * auth (void *arg) {
    int clnt_sock = *((int *)arg);
    free(arg);
 
    pthread_t *tid_ptr = (pthread_t *)malloc(sizeof(pthread_t));
    *tid_ptr = pthread_self();
    pthread_cleanup_push(cleanup_handler2_auth, tid_ptr);

    // send map data to clnt_sock
    
    printf("PID[auth] : %lx\n", pthread_self());
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
        if (pthread_create(&tid, NULL, send_mapdata, (void *)clnt_sock) != 0) {
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
        if (pthread_create(&tid, NULL, auth, (void *)clnt_sock) != 0) {
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
 * Create one thread for each task:
 *   1. Client connection manager — sends map data
 *   2. Client connection manager — receives ID/password
 *   3. Camera interface
 *   4. ISP processing
 *   5. Device driver — sensor
 *   6. Device driver — actuator
 *
 * Returns 0 on success, 1 on failure.
 */
int setting_threads (void) {

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
    
    /*
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
        */
        
    return 0;
}

/*
 * Run ai-engine
 * Return 0 on success, 1 on failure
 */
int setting_process (void) {
    ai_pid = fork();
    if (ai_pid == 0) {      /* child */
        // Execute other program or call function
        while (1) {
            sleep(1);
        }
    }
    else if (ai_pid > 0) {  /* parent */
        fprintf(stdout, "Process %d spawned process %d\n", getpid(), ai_pid) ;
    }
    else {      /* ai_pid < 0 */
        fprintf(stderr, "Fork failed\n");
        return 1;
    }

    return 0;
}

/*
 * Performs cleanup tasks to ensure a safe exit when the server is forcibly terminated
 */
void exit_routine() {
    // Terminate the threads[]
    int i;
    for (i = 1; i < 3; i++) {
        if (pthread_cancel(threads[i]) != 0) {
            fprintf(stderr, "%d's pthread_cancel failure\n", i);
        }
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "%d's pthread_join failure\n", i);
        }
        fprintf(stderr, "Terminate the threads[%d]\n", i);
    } 

    // Terminate the ai-process
    if (kill(ai_pid, SIGTERM) == -1) {
        perror("Error: ai_pid sigterm failed");
    }
    int wstatus;
    if (waitpid(ai_pid, &wstatus, 0) != ai_pid) {
        perror("Error: ai_pid waitpid failed");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "Terminate the ai_pid\n");
}

int main (void) {

    if (setting_process() == 1) {
        fprintf(stderr, "setting_process() failure\n");
        exit(EXIT_FAILURE);
    }

    if (setting_threads() == 1) {
        fprintf(stderr, "setting_threads() failed\n");
        exit(EXIT_FAILURE);
    }

    int input;
    printf(">> ");
    scanf("%d", &input);

    if (input < 0) {
        exit_routine();
    }

    return 0;

}