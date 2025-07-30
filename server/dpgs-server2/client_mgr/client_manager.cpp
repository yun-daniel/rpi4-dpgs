#include "client_manager.h"


ClientManager::ClientManager (MapManager& _map_mgr, StrFrameBuffer& _fb)
    : map_mgr(_map_mgr), fb(_fb) {
    is_running = false;
    is_updated = false;
}


bool ClientManager::initialize() {
    std::cout << "[CLT_MGR] Start to initialize...\n";

    updated_mutex   = PTHREAD_MUTEX_INITIALIZER;
    updated_cv      = PTHREAD_COND_INITIALIZER;

    if (!conn_mgr.initialize(this, &fb)) {
        std::cout << "[CLT_MGR] Error: Failed to initialize Connection Manager\n";
        return false;
    }
    if (!map_mon.initialize(this, &conn_mgr, &map_mgr)) {
        std::cout << "[CLT_MGR] Error: Failed to initialize Map Monitor\n";
        return false;
    }

    sfa.clnt_mgr_ptr = this;
    sfa.conn_mgr_ptr = &conn_mgr;

    std::cout << "[CLT_MGR] Success: Client Manager initialized\n";
    return true;
}


void ClientManager::run() {
    std::cout << "[CLT_MGR] Start Client Manager\n";

    is_running = true;

    if (pthread_create(&tid_arr[0], NULL, ClientManager::handle_ConnMgr_run, this)) {
        return;
    }

    if (pthread_create(&tid_arr[1], NULL, ClientManager::handle_MapMon_run, this)) {
        return;
    }
    // [Debug Session]
    // Dummy Client Manager
//    is_running = true;
//    while (is_running) {
//        cv::Mat sampled = vp_engine.clt_fb1->pop();
//        if (sampled.empty()) {
//            continue;
//        }
//        cv::Mat cloned = sampled.clone();
//        if (cloned.empty()) {
//            continue;
//        }

//        std::cout << "[CLT_MGR][DEBUG] Frame Info: rows: " << cloned.rows << " cols: " << cloned.cols << " size: " << cloned.size << "\n";

//        cv::imshow("STRM", cloned);
//        cv::waitKey(1);
//    }
    // ---------------------------

}

void ClientManager::stop() {
    std::cout << "[CLT_MGR] Client Manager Terminating...\n";

    if (pthread_cancel(tid_arr[0]) != 0) {
        fprintf(stderr, "[CLT_MGR] Error: %lx pthread_cancel failure\n", tid_arr[0]);
    }

    if (pthread_cancel(tid_arr[1]) != 0) {
        fprintf(stderr, "[CLT_MGR] Error: %lx pthread_cancel failure\n", tid_arr[1]);
    }

    conn_mgr.stop();
    map_mon.stop();

    clear();
    is_running = false;

    std::cout << "[CLT_MGR] Client Manager Terminated\n";
}


void ClientManager::clear () {
    std::cout << "[CLT_MGR] clear: Cleanning...\n";

    pthread_join(tid_arr[0], NULL);
    pthread_join(tid_arr[1], NULL);

    pthread_mutex_destroy(&updated_mutex);
    pthread_cond_destroy(&updated_cv);

    std::cout << "[CLT_MGR] clear: Cleanning Success\n";
}


SharedParkingLotMap* ClientManager::get_mapdata() {
    return &mapdata;
}

bool* ClientManager::get_is_updated() {
    return &is_updated;
}

pthread_mutex_t* ClientManager::get_updated_mutex() {
    return &updated_mutex;
}

pthread_cond_t* ClientManager::get_updated_cv() {
    return &updated_cv;
}


// Client Manager Thread Handler
void* ClientManager::handle_ConnMgr_run(void* arg) {
    ClientManager* clt_mgr_ptr = (ClientManager*)arg;

    clt_mgr_ptr->conn_mgr.run();

    return nullptr;
}

void* ClientManager::handle_MapMon_run(void* arg) {
    ClientManager* clt_mgr_ptr = (ClientManager*)arg;

    clt_mgr_ptr->map_mon.run();

    return nullptr;
}

