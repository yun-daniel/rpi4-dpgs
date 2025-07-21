#include "client_manager.h"
#include "map_monitor.h"
#include "client_info.h"


bool MapMonitor::initialize(ClientManager* _clnt_mgr_ptr, ConnectionManager* _conn_mgr_ptr, MapManager* _map_mgr) {

    if (_clnt_mgr_ptr == nullptr) {
        fprintf(stderr, "Failed to connect clt_mgr\n");
        return false;
    }
    else {
        clnt_mgr_ptr = _clnt_mgr_ptr;
    }

    if (_conn_mgr_ptr == nullptr) {
        fprintf(stderr, "Failed to connect conn_mgr\n");
        return false;
    }
    else {
        conn_mgr_ptr = _conn_mgr_ptr;
    }

    if (_map_mgr == nullptr) {
        fprintf(stderr, "Failed to connect map_mgr\n");
        return false;
    }
    else {
        map_mgr = _map_mgr;
    }


    return true;
}

void MapMonitor::stop () {
    std::cout << "[MAP_MON] Map Monitor Terminating...\n";

    is_running = false;
    pthread_mutex_lock(map_mgr->get_mutex_clt());
    pthread_cond_signal(map_mgr->get_cv_clt());
    pthread_mutex_unlock(map_mgr->get_mutex_clt());

}


void MapMonitor::run() {
    std::cout << "[MAP_MON] Map Monitor Running...\n";

    /* ClientManager */
    SharedParkingLotMap* mapdata_ptr = clnt_mgr_ptr->get_mapdata();
    bool * is_updated_ptr = clnt_mgr_ptr->get_is_updated();     
    pthread_mutex_t * updated_mutex_ptr = clnt_mgr_ptr->get_updated_mutex();   
    pthread_cond_t * updated_cv_ptr = clnt_mgr_ptr->get_updated_cv();

    /* ConnectionManager */
    std::vector<ClientInfo>* client_info_vec_ptr = conn_mgr_ptr->get_client_info_vec();
    pthread_mutex_t* client_info_vec_mutex_ptr = conn_mgr_ptr->get_client_info_vec_mutex();
    pthread_cond_t* all_sent_cv_ptr = conn_mgr_ptr->get_all_sent_cv();


    SharedParkingLotMap updated_map;

    is_running = true;
    while (is_running) {

        pthread_mutex_lock(map_mgr->get_mutex_clt());
        while ((!*(map_mgr->get_flag_ptr_clt())) && is_running) {
            std::cout << "[MAP_MON][DEBUG] MAPMON Wait...\n";
            pthread_cond_wait(map_mgr->get_cv_clt(), map_mgr->get_mutex_clt());
        }
        updated_map = map_mgr->getMap();
	std::cout << "[MAP_MON][DEBUG] MAP_MON flag: " << *(map_mgr->get_flag_ptr_clt()) << " and deflag...\n";
        *(map_mgr->get_flag_ptr_clt()) = false;
        pthread_mutex_unlock(map_mgr->get_mutex_clt());
        if (!is_running) break;


        std::vector<pthread_t> targets;
        ClientInfo ci;
        pthread_mutex_lock(client_info_vec_mutex_ptr);
        pthread_cleanup_push(unlock_mutex, (void*)client_info_vec_mutex_ptr);
        for (auto &ci : *client_info_vec_ptr) {
            ci.set_sent_map_flag(false);
            targets.push_back(ci.get_tid());
        }
        pthread_cleanup_pop(1);


        pthread_mutex_lock(updated_mutex_ptr);
        // Updated Client Manager map
        *mapdata_ptr = updated_map;
        *is_updated_ptr = true;
        pthread_cond_broadcast(updated_cv_ptr);
        pthread_mutex_unlock(updated_mutex_ptr);


        pthread_mutex_lock(client_info_vec_mutex_ptr);
        pthread_cleanup_push(unlock_mutex, (void*)client_info_vec_mutex_ptr);
        while (1) {
            bool all_sent = true;
            for (auto tid : targets) {
                auto it = conn_mgr_ptr->find_client(tid);
                if (it == (*client_info_vec_ptr).end()) {
                    continue;
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

        pthread_mutex_lock(updated_mutex_ptr);
        *is_updated_ptr = false;
        pthread_mutex_unlock(updated_mutex_ptr);

    }

    std::cout << "[MAP_MON] Map Monitor Terminated\n";

}


