#include "client_thread_manager.h"

void * ClientThreadManager::send_mapdata(void *arg) {
    pthread_t *tid_ptr = (pthread_t *)malloc(sizeof(pthread_t));
    *tid_ptr = pthread_self();
    pthread_cleanup_push(remove, tid_ptr);

    int clnt_sock = *(int*)arg;
    free(arg);

    while (1) {
        pthread_testcancel();

        // Wait for signal
        pthread_mutex_lock(&cond_mutex);
            while (mapdata_ready == false) {
                pthread_cond_wait(&cond, &cond_mutex);
            }
        pthread_mutex_unlock(&cond_mutex);

        // no lock here → assuming mapdata is read-only & safe
        // send map data : send(clnt_sock, ..);

        pthread_mutex_lock(&cond_mutex);
            sent_counter++;
            if (sent_counter == temp_total_clients) {
                mapdata_ready = false;
                pthread_cond_broadcast(&cond);
            }
        pthread_mutex_unlock(&cond_mutex);
    }

    pthread_cleanup_pop(1);

    return nullptr;
}

ClientThreadManager::ClientThreadManager() {
    mapdata_ready = false;

    cond_mutex = PTHREAD_MUTEX_INITIALIZER;
    m1 = PTHREAD_MUTEX_INITIALIZER;
    
    cond = PTHREAD_COND_INITIALIZER;
}

/*
 * make listen_fd, bind + listen
 * Returns 0 on success, 1 on failure.
 */
int ClientThreadManager::initialize() {
    return 0;
}

/*
 * 
 */
void ClientThreadManager::connect_client() {
    int *clnt_sock;
    pthread_t tid;
    while (1) {
        pthread_testcancel();
        clnt_sock = (int *)malloc(sizeof (int));
        // *clnt_sock = accept(listen_fd, ...);

        // add tid;
        if (pthread_create(&tid, NULL, send_mapdata, (void *)clnt_sock) != 0) {
            fprintf(stderr, "Error: %d's pthread_create of mapdata failed\n", *clnt_sock);
            free (clnt_sock);
        }
        else {
            pthread_mutex_lock(&m1);
                client_tid_vec.push_back(tid);
            pthread_mutex_unlock(&m1);
        }

        sleep(3);
    }
}

// cleanup handler for send_mapdata()
void ClientThreadManager::remove(void *arg) {
    pthread_t tid = *((pthread_t *)arg);
    free (arg);

    pthread_mutex_lock(&m1);
        auto it = find(client_tid_vec.begin(), client_tid_vec.end(), tid);
        if (it == client_tid_vec.end()) {
            fprintf(stderr, "Find(cleanup_handler2_mapdata) failure\n");
            pthread_mutex_unlock(&m1);
            return;
        }
        fprintf(stderr, "Erase(mapdata) : %lx\n", tid);
        client_tid_vec.erase(it);
    pthread_mutex_unlock(&m1);
}

int ClientThreadManager::clear() {

    vector<pthread_t> v_copy;
    pthread_mutex_lock(&m1);
        v_copy = client_tid_vec;
        for (pthread_t tid : client_tid_vec) {
            if (pthread_cancel(tid) != 0) {
                fprintf(stderr, "Error: %lx pthread_cancel failure\n", tid);
            }
        }
    fprintf(stderr, "Clear in client_thread_manager: %lx\n", pthread_self());
    pthread_mutex_unlock(&m1);

    int ret;
    for (pthread_t tid : v_copy) {
        ret = pthread_join(tid, NULL);
        if (ret == ESRCH) {
            fprintf(stderr, "%lx is already deleted\n", tid);
        }
    }

    return 0;
}

// This function will be executed not concurrently
int ClientThreadManager::siganl_mapdata() {

    pthread_mutex_lock(&m1);
        sent_counter = 0;
        temp_total_clients = client_tid_vec.size();
    pthread_mutex_unlock(&m1);

    // read mapdata using mapmanager : mapdata = ..

    pthread_mutex_lock(&cond_mutex);
        mapdata_ready = true;
        pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&cond_mutex);

    // check false and return value
    pthread_mutex_lock(&cond_mutex);
        while (mapdata_ready == true) {
           pthread_cond_wait(&cond, &cond_mutex); 
        }
    pthread_mutex_unlock(&cond_mutex);

    return 0;
}