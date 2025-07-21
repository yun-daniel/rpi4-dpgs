#include "map_monitor.h"
#include "client_manager.h" 
#include "client_info.h"

int MapMonitor::initialize () {

    return 0;
}

int MapMonitor::stop () {

    return 0;
}

/* Static Function */
void * MapMonitor::run (void * arg) {
    SFA * sfa_ptr = (SFA *)arg;  
    ClientManager * clnt_mgr_ptr = sfa_ptr->clnt_mgr_ptr;
    ConnectionManager * conn_mgr_ptr = sfa_ptr->conn_mgr_ptr;

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

    /* ConnectionManager */
    vector<ClientInfo> * client_info_vec_ptr = conn_mgr_ptr->get_client_info_vec();
    pthread_mutex_t * client_info_vec_mutex_ptr = conn_mgr_ptr->get_client_info_vec_mutex();
    pthread_cond_t * all_sent_cv_ptr = conn_mgr_ptr->get_all_sent_cv();

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

        sleep(5);
        
        // Copy map data

        // Set the targets(client list that have to send mapdata)
        vector<pthread_t> targets;
        ClientInfo ci;
        pthread_mutex_lock(client_info_vec_mutex_ptr);
            pthread_cleanup_push(unlock_mutex, (void *)client_info_vec_mutex_ptr);
            for (auto &ci : *client_info_vec_ptr) {
                ci.set_sent_map_flag(false);
                targets.push_back(ci.get_tid());
            }
        pthread_cleanup_pop(1);

        // Update mapdata and is_updated 
        // and broadcast to updated_cv in ClientManager
        pthread_mutex_lock(updated_mutex_ptr);
            *mapdata_ptr = 10;
            *is_updated_ptr = true;
            pthread_cond_broadcast(updated_cv_ptr);
        pthread_mutex_unlock(updated_mutex_ptr);

        // Check if all clients send mapdata
        pthread_mutex_lock(client_info_vec_mutex_ptr);
            pthread_cleanup_push(unlock_mutex, (void *)client_info_vec_mutex_ptr);
            while (1) {
                bool all_sent = true;
                for (auto tid : targets) {
                    auto it = conn_mgr_ptr->find_client(tid);
                    if (it == (*client_info_vec_ptr).end()) {
                        continue;   // Ignore the erased client
                    }
                    if (it->get_sent_map_flag() == false) {
                        all_sent = false;
                        break;
                    }
                }

                if (all_sent == true) {
                    break;
                }

                pthread_cond_wait(all_sent_cv_ptr, client_info_vec_mutex_ptr);
            }
        pthread_cleanup_pop(1);

        // Change is_updated to false
        pthread_mutex_lock(updated_mutex_ptr);
            *is_updated_ptr = false;
        pthread_mutex_unlock(updated_mutex_ptr);

        // Update flag_map_clt in MapManager to false
        flag_map_clt = false;
    }

    return nullptr;
}