#include "client_manager.h"

int ClientManager::initialize () {
    is_running = true;
    is_updated = false;
    updated_mutex = PTHREAD_MUTEX_INITIALIZER;
    updated_cv = PTHREAD_COND_INITIALIZER;

    conn_mgr.initialize();
    // map_mon.initialize();

    sfa.clnt_mgr_ptr = this;
    sfa.conn_mgr_ptr = &conn_mgr;

    return 0;
}

int ClientManager::stop () {
    is_running = false;
    
    if (pthread_cancel(tid_arr[0]) != 0) {
        fprintf(stderr, "Error: %lx pthread_cancel failure\n", tid_arr[0]);
    }

    conn_mgr.stop();
    // map_mon.stop();


    return 0;
}

int ClientManager::clear () {

    pthread_join(tid_arr[0], NULL);
    // pthread_join(tid_arr[1], NULL);

    pthread_mutex_destroy(&updated_mutex);
    pthread_cond_destroy(&updated_cv);

    return 0;
}

int * ClientManager::get_mapdata () {
    return &mapdata;
}

bool * ClientManager::get_is_updated () {
    return &is_updated;
}

pthread_mutex_t * ClientManager::get_updated_mutex () {
    return &updated_mutex;
}

pthread_cond_t * ClientManager::get_updated_cv () {
    return &updated_cv;
}

void * ClientManager::run (void * arg) {

    ClientManager * clnt_mgr_ptr = (ClientManager *)arg;

    if (pthread_create(&clnt_mgr_ptr->tid_arr[0], NULL, ConnectionManager::run, &clnt_mgr_ptr->sfa)) {
    }
    // if (pthread_create(&tid_arr[1], NULL, MapMonitor::run(), (void *)this)) {
    //     return 1;
    // }

    // is this necessary..?
    while (clnt_mgr_ptr->is_running == true) {
    }

    printf("ClientManager Run End\n");

    return nullptr;
}