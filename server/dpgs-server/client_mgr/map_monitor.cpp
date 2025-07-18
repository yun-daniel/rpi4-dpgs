#include "map_monitor.h"
#include "client_manager.h" 

bool MapMonitor::initialize (MapManager& _map_mgr) {

    return true;
}

void MapMonitor::stop () {

}

/* Static Function */
void * MapMonitor::run (void * arg) {
    ClientManager * clnt_mgr_ptr = (ClientManager *)arg;

    /* TEST */
    bool flag_map_clt = true;
    pthread_mutex_t mutex_map_clt;
    pthread_cond_t  cv_map_clt;
    int mapdata;
    /* TEST */

    /* ClientManager */
    int * mapdata_ptr = clnt_mgr_ptr->get_mapdata();
    bool * is_updated_ptr = clnt_mgr_ptr->get_is_updated();     
    pthread_mutex_t * updated_mutex_ptr = clnt_mgr_ptr->get_updated_mutex();   
    pthread_cond_t * updated_cv_ptr = clnt_mgr_ptr->get_updated_cv();


    /*
    // check flag_map_clt using map_mgr
    mutex_lock:mutex_map_clt
    while (flag_map_clt == true) {
        cond_wait(cv_map_clt)

        // copy map data
        get_data(); 

        // change is_updated in ClientManager to true
        mutex_unlock:mutex_map_clt;

        // Broadcast to cond
    }
        */
    
    while (flag_map_clt == true) {

        // Update mapdata and is_updated 
        // and broadcast to updated_cv in ClientManager
        pthread_mutex_lock(updated_mutex_ptr);
            *mapdata_ptr = 10;
            *is_updated_ptr = true;
            pthread_cond_broadcast(updated_cv_ptr);
        pthread_mutex_unlock(updated_mutex_ptr);

        // Update flag_map_clt in MapManager to false
        flag_map_clt = false;
    }

    return nullptr;
}
