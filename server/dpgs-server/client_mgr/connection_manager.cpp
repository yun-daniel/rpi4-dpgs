#include "connection_manager.h"
#include "client_manager.h"

typedef struct ClientThreadArgs {
    int clnt_sock;
//    ClientManager * clnt_mgr_ptr;
    ConnectionManager * conn_mgr_ptr;
//    VPEngine*       vp_engine_ptr;
} CTA;

typedef struct RemoveData {
    StreamingModule * sm_ptr;
    SFA * sfa_ptr;
    pthread_t tid_arr[2];       // tid_arr[2] in client thread
                                // tid_arr[0] : SEND MAP
                                // tid_arr[1] : STREAMING
} RD;

bool ConnectionManager::initialize(ClientManager* _clt_mgr, VPEngine* _vp_engine) {
    std::cout << "[CNT_MGR] Start to initialize...\n";

    if (_clt_mgr == nullptr) {
        fprintf(stderr, "Failed to connect clt_mgr\n");
        return false;
    }
    else {
        clt_mgr_ptr = _clt_mgr;
    }

    if (_vp_engine == nullptr) {
        fprintf(stderr, "Failed to connect vp_engine\n");
        return false;
    }
    else {
        vp_engine_ptr = _vp_engine;
    }

    port = 9999;

    client_info_vec_mutex = PTHREAD_MUTEX_INITIALIZER;
    empty_cv = PTHREAD_COND_INITIALIZER;
    all_sent_cv = PTHREAD_COND_INITIALIZER;

    // Setting detach option of pthread
    if (pthread_attr_init(&attr) != 0) {
        fprintf(stderr, "pthread_attr_init failure\n");
    }
    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
        fprintf(stderr, "pthread_attr_setdetachstate failure\n");
    }

    // Initialize OpenSSL
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    ctx = SSL_CTX_new(TLS_server_method());
    if (SSL_CTX_use_certificate_file(ctx, CERT_FILE_PATH, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        return false;
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, KEY_FILE_PATH, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        return false;
    }

    // Make new socket to listen and connect with port
    listen_fd = socket(AF_INET /*IPv4*/, SOCK_STREAM /*TCP*/, 0 /*IP*/) ;
	if (listen_fd == 0)  { 
		perror("socket failed : "); 
		return false;
	}

    int optval = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt(SO_REUSEADDR)");
        return false;
    }
	
	memset(&address, '0', sizeof(address)); 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY /* the localhost*/ ; 
	address.sin_port = htons(port); 
	if (bind(listen_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("bind failed : "); 
		return false;
	} 

    if (listen(listen_fd, 16 /* the size of waiting queue*/) < 0) { 
        perror("listen failed : "); 
        return false;
    } 

    std::cout << "[CNT_MGR] Success: Connection Manager initialized\n";
    return true;
}


void ConnectionManager::run() {
    std::cout << "[CNT_MGR] Connection Manager Running...\n";

    int clnt_sock;
    pthread_t tid;
    CTA* cta_ptr;

    is_running = true;
    while (is_running) {
        if ((clnt_sock = accept(listen_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Error: accept");
            continue;
        }

        std::cout << "[CNT_MGR] Connected clt_sock_ptr: " << clnt_sock << "\n";

        cta_ptr = (CTA*)malloc(sizeof(CTA));
        cta_ptr->clnt_sock = clnt_sock;
        cta_ptr->conn_mgr_ptr = this;
        if (pthread_create(&tid, &attr, handle_client_thread, (void*)cta_ptr) != 0) {
            fprintf(stderr, "Error: %d's pthread_create of client_thread failed\n", clnt_sock);
            free(cta_ptr);
        }
    }

    std::cout << "[CNT_MGR] Connection Manager Stopped\n";

}


void ConnectionManager::stop () {
    
    // Cancels every client thread, waits until client_info_vec is empty
    pthread_t tid;
    pthread_mutex_lock(&client_info_vec_mutex);
        for (auto it : client_info_vec) {
            tid = it.get_tid();
            if (pthread_cancel(tid) != 0) {
                fprintf(stderr, "Error: %lx pthread_cancel failure\n", tid);
            }
            SSL_shutdown(it.get_ssl());
        }
        fprintf(stderr, "Waiting detach(remove) for stop in connection manager...\n");

        while (client_info_vec.empty() != true) {
            printf("Cond waiting...\n");
            pthread_cond_wait(&empty_cv, &client_info_vec_mutex);
        }
    pthread_mutex_unlock(&client_info_vec_mutex);

    printf("Cond waiting done..\n");

    clear();

}

void ConnectionManager::clear () {
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&client_info_vec_mutex);
    pthread_cond_destroy(&empty_cv);
    pthread_cond_destroy(&all_sent_cv);
    SSL_CTX_free(ctx); 
    
}

vector<ClientInfo>::iterator ConnectionManager::find_client (pthread_t tid) {
    return std::find_if(client_info_vec.begin(), client_info_vec.end(),
        [tid](ClientInfo& ci) {
            return pthread_equal(ci.get_tid(), tid);
        });
}


void ConnectionManager::exec_client_thread(int clnt_sock) {

    std::cout << "[CNT_MGR][CLT " << clnt_sock << "] pthread: " << pthread_self() << "\n";


    SFA* sfa_ptr = (SFA*)malloc(sizeof(SFA));
    sfa_ptr->clnt_mgr_ptr = clt_mgr_ptr;
    sfa_ptr->conn_mgr_ptr = this;
    sfa_ptr->tid = pthread_self();

    RD* rd_ptr = (RD*)malloc(sizeof(RD));
    rd_ptr->sm_ptr = new StreamingModule(*vp_engine_ptr);
    rd_ptr->sfa_ptr = sfa_ptr;

    SSL * ssl = SSL_new(ctx);
    SSL_set_fd(ssl, clnt_sock);
    SSL_accept(ssl);

    ClientInfo ci;
    ci.set_sent_map_flag(false);
    ci.set_sock_fd(clnt_sock);
    ci.set_tid(pthread_self());
    ci.set_ssl(ssl);

    pthread_mutex_lock(&client_info_vec_mutex);
    client_info_vec.push_back(ci);
    pthread_mutex_unlock(&client_info_vec_mutex);
    std::cout << "[CNT_MGR][DEBUG] client_info pushed: clnt_sock=" << clnt_sock << ", tid=" << pthread_self() << "\n";

    pthread_cleanup_push(remove, (void*)rd_ptr);

    if (check_idpw(ssl) == 1) {
        fprintf(stderr, "Error: check_idpw failed\n");
        pthread_exit(NULL);
    }

    if (pthread_create(&rd_ptr->tid_arr[0], NULL, send_mapdata, (void*)sfa_ptr) != 0) {
        fprintf(stderr, "Error: %d's pthread_create of send_mapdata failed\n", clnt_sock);
        pthread_exit(NULL);
    }

    if (pthread_create(&rd_ptr->tid_arr[1], NULL, streaming, (void*)rd_ptr->sm_ptr) != 0) {
        fprintf(stderr, "Error: %d's pthread_create of rtsp failed\n", clnt_sock);
        pthread_exit(NULL);
    }

    std::cout << "[CNT_MGR] " << clnt_sock << " spawned tid_arr[0]: " << rd_ptr->tid_arr[0] << "\n";
    std::cout << "[CNT_MGR] " << clnt_sock << " spawned tid_arr[1]: " << rd_ptr->tid_arr[1] << "\n";

    if (recv_msg(clnt_sock, ssl, rd_ptr->sm_ptr) == 1) {
        fprintf(stderr, "Error: recv_msg failed\n");
    }

    pthread_cleanup_pop(1);

}


/* Static Function */
void ConnectionManager::remove (void * arg) {
    
        RD * rd_ptr = (RD *)arg;
        ClientManager * clnt_mgr_ptr = rd_ptr->sfa_ptr->clnt_mgr_ptr;
        ConnectionManager * conn_mgr_ptr = rd_ptr->sfa_ptr->conn_mgr_ptr; 
        pthread_t tid_arr[2];
        memcpy(tid_arr, rd_ptr->tid_arr, sizeof(pthread_t) * 2);

        // Cancel and join the two worker threads (map-sender and RTSP)
        int ret;
        for (pthread_t tid : tid_arr) {
            ret = pthread_cancel(tid);
            if (ret == ESRCH) {
                fprintf(stderr, "Warning: thread %lx does not exist (cancel returned ESRCH)\n", tid);
            }
            else if (ret != 0) {
                fprintf(stderr, "Error: %lx pthread_cancel failed\n", tid);
            }
            std::cout << "[CNT_MGR][DEBUG] pthread_cancel: tid=" << tid << " done\n";

            ret = pthread_join(tid, NULL);
            if (ret == ESRCH) {
                fprintf(stderr, "Warning: thread %lx does not exist (join returned ESRCH)\n", tid);
            }
            else if (ret != 0) {
                fprintf(stderr, "Error: %lx pthread_join failed\n", tid);
            }
            std::cout << "[CNT_MGR][DEBUG] pthread_join: tid=" << tid << " done\n";
        }

        printf("REMOVE: spawned %lx\t%lx\n", tid_arr[0], tid_arr[1]);

        // Erase this client’s record from client_info_vec
        pthread_mutex_lock(&conn_mgr_ptr->client_info_vec_mutex);
        auto it = conn_mgr_ptr->find_client(pthread_self());
        if (it == conn_mgr_ptr->client_info_vec.end()) {
            fprintf(stderr, "Error: Find(remove) failed\n");
            pthread_mutex_unlock(&conn_mgr_ptr->client_info_vec_mutex);
            return;
        }

        fprintf(stderr, "REMOVE: %lx\n", it->get_tid());
        close (it->get_sock_fd());
        SSL_free (it->get_ssl());
        conn_mgr_ptr->client_info_vec.erase(it);

        pthread_cond_signal(&conn_mgr_ptr->all_sent_cv);

        if (conn_mgr_ptr->client_info_vec.empty() == true) {
            pthread_cond_signal(&conn_mgr_ptr->empty_cv);
        }
        pthread_mutex_unlock(&conn_mgr_ptr->client_info_vec_mutex);

        // Free per-client resources
        delete rd_ptr->sm_ptr;
        free (rd_ptr->sfa_ptr);
        free (arg);     
}

/* Static Function */
void * ConnectionManager::send_mapdata (void * arg) {

        SFA * sfa_ptr = (SFA *)arg;
        ClientManager * clnt_mgr_ptr = sfa_ptr->clnt_mgr_ptr;
        ConnectionManager * conn_mgr_ptr = sfa_ptr->conn_mgr_ptr;  

        /* ClientManager */
        SharedParkingLotMap* mapdata_ptr = clnt_mgr_ptr->get_mapdata();
        bool* is_updated_ptr = clnt_mgr_ptr->get_is_updated();
        pthread_mutex_t* updated_mutex_ptr = clnt_mgr_ptr->get_updated_mutex();
        pthread_cond_t* updated_cv_ptr = clnt_mgr_ptr->get_updated_cv();

        /* ConnectionManager */
        std::vector<ClientInfo>* client_info_vec_ptr = conn_mgr_ptr->get_client_info_vec();
        pthread_mutex_t* client_info_vec_mutex_ptr = conn_mgr_ptr->get_client_info_vec_mutex();
        pthread_cond_t* all_sent_cv_ptr = conn_mgr_ptr->get_all_sent_cv();


        while (1) {
            pthread_testcancel();

            // Check the client already sent mapdata
            pthread_mutex_lock(client_info_vec_mutex_ptr);
            // pthread_cleanup_push(unlock_mutex, (void *)client_info_vec_mutex_ptr);
            auto it = conn_mgr_ptr->find_client(sfa_ptr->tid);
            int clnt_sock = it->get_sock_fd();
            SSL * ssl = it->get_ssl();
            if (it == (*client_info_vec_ptr).end()) {
                fprintf(stderr, "Error: Find(send_mapdata) failed\n");      // This will be never printed
                pthread_mutex_unlock(client_info_vec_mutex_ptr);
                return nullptr;
            }
            if (it->get_sent_map_flag() == true) {
                pthread_mutex_unlock(client_info_vec_mutex_ptr);
                continue;
            }
            pthread_mutex_unlock(client_info_vec_mutex_ptr);

            // Wait for updated signal
            pthread_mutex_lock(updated_mutex_ptr);
            pthread_cleanup_push(unlock_mutex, (void*)updated_mutex_ptr);
            while (*is_updated_ptr == false) {
                pthread_cond_wait(updated_cv_ptr, updated_mutex_ptr);
            }
            pthread_cleanup_pop(1);

            // No lock here -> assuming mapdata is read-only & safe
            // Send map data : send(clnt_sock, ...)
            if (send_bytes(ssl, mapdata_ptr->slots, sizeof(Slot) * SLOTS_MAX_SIZE) == -1) {
                fprintf(stderr, "Error: %d's send error\n", clnt_sock);
                pthread_exit(NULL);
            }
        std::cout << "[CNT_MGR][DEBUG] Updated Map Sended\n";

            // Change sent_flag to true
            pthread_mutex_lock(client_info_vec_mutex_ptr);
            pthread_cleanup_push(unlock_mutex, (void *)client_info_vec_mutex_ptr);
            it = conn_mgr_ptr->find_client(sfa_ptr->tid);
            if (it == (*client_info_vec_ptr).end()) {
                fprintf(stderr, "Error: Find(send_mapdata) failed\n");      // This will be never printed
                pthread_mutex_unlock(client_info_vec_mutex_ptr);
                return nullptr;
            }
            it->set_sent_map_flag(true);
            pthread_cleanup_pop(1);

            pthread_cond_signal(all_sent_cv_ptr);
        }

    return nullptr;
}

/* Static Function */
void * ConnectionManager::streaming (void * arg) {

    StreamingModule* sm_ptr = (StreamingModule*)arg;
    sm_ptr->run();

    return nullptr;
}


// Connection Manager Thread Handler
void* ConnectionManager::handle_client_thread(void* arg) {
    CTA* cta_ptr = (CTA*)arg;
    int clnt_sock = cta_ptr->clnt_sock;
    ConnectionManager * conn_mgr_ptr = cta_ptr->conn_mgr_ptr;
    free (arg);

    conn_mgr_ptr->exec_client_thread(clnt_sock);

    return nullptr;
}


// Interface
std::vector<ClientInfo>* ConnectionManager::get_client_info_vec() {
    return &client_info_vec;
}

pthread_mutex_t* ConnectionManager::get_client_info_vec_mutex() {
    return &client_info_vec_mutex;
}

pthread_cond_t* ConnectionManager::get_all_sent_cv() {
    return &all_sent_cv;
}

