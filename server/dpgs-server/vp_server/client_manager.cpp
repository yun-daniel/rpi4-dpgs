#include "client_manager.h"

ClientManager* ClientManager::cm_ptr = nullptr;

/*
 * Parameter structure passed to rtsp().
 *   cam_rq   : Camera index requested by the client.
 *   m_cam_rq : Mutex for cam_rq.
 */
typedef struct RtspData {
    int cam_rq;                 // Cam number that received from client
    pthread_mutex_t m_cam_rq;   // Mutex for cam_rq
} RTD;


typedef struct RtspEndData {
    bool init_flag;                 
    pthread_t tid;   
} RED;
/*
 * Parameter structure passed to remove().
 *   rtd_ptr : Pointer to the RtspData.
 *   tid_arr : IDs of the two worker threads spawned by the client thread
 *             tid_arr[0] – map-sending thread
 *             tid_arr[1] – RTSP thread
 */
typedef struct RemoveData {
    RTD * rtd_ptr;
    pthread_t tid_arr[2];       // tid_arr[2] in client thread
                                // tid_arr[0] : MAP SEND
                                // tid_arr[1] : RTSP
} RD;

typedef struct StreamingModuleData {
    StreamingModule streaming_module_;
    int cam_id_;
} SMD;


ClientManager::ClientManager (int _port) : port(9999) {
    port = _port;
    updated = false;
    m_client_info_vec = PTHREAD_MUTEX_INITIALIZER;
    m_updated = PTHREAD_MUTEX_INITIALIZER;
    cond_clear = PTHREAD_COND_INITIALIZER;
    cond_updated = PTHREAD_COND_INITIALIZER;
    cond_all_sent = PTHREAD_COND_INITIALIZER;
    // Setting detach option of pthread
    if (pthread_attr_init(&attr) != 0) {
        fprintf(stderr, "pthread_attr_init failure\n");
    }
    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
        fprintf(stderr, "pthread_attr_setdetachstate failure\n");
    }
}

/*
 * Make new socket to listen and connect with port.
 * Returns 0 on success, 1 on failure. 
 */
int ClientManager::initialize () {
    listen_fd = socket(AF_INET /*IPv4*/, SOCK_STREAM /*TCP*/, 0 /*IP*/) ;
	if (listen_fd == 0)  { 
		perror("socket failed : "); 
		return 1;
	}

    int optval = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt(SO_REUSEADDR)");
        return 1;
    }
	
	memset(&address, '0', sizeof(address)); 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY /* the localhost*/ ; 
	address.sin_port = htons(port); 
	if (bind(listen_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("bind failed : "); 
		return 1;
	} 

    if (listen(listen_fd, 16 /* the size of waiting queue*/) < 0) { 
        perror("listen failed : "); 
        return 1;
    } 

    return 0;
}

/*
 * Connect client and spawn thread each.
 */
void ClientManager::connect_client () {

    ClientInfo ci;
    int clnt_sock;
    int * clnt_sock_ptr;
    pthread_t tid;
    
    while (1) {
        pthread_testcancel();
        if ((clnt_sock = accept(listen_fd, (struct sockaddr *) &address, (socklen_t*)&addrlen)) < 0) {
            perror("Error: accept");
            continue;
        }
        printf("Connected clnt_sock_ptr : %d\n", clnt_sock);
        
        // clnt_sock_ptr = (int *)malloc(sizeof(int));
        // *clnt_sock_ptr = i++;
    
        // add tid;
        clnt_sock_ptr = (int *)malloc(sizeof(int));
        *clnt_sock_ptr = clnt_sock;
        if (pthread_create(&tid, &attr, client_thread, (void *)clnt_sock_ptr) != 0) {
            fprintf(stderr, "Error: %d's pthread_create of client_thread failed\n", *clnt_sock_ptr);
            free (clnt_sock_ptr);
        }
        else {
            ci.set_sent_map_flag(false);
            ci.set_sock_fd(clnt_sock);
            ci.set_tid(tid);
            pthread_mutex_lock(&m_client_info_vec);
                pthread_cleanup_push(unlock_mutex, (void *)&m_client_info_vec);
                client_info_vec.push_back(ci);
            pthread_cleanup_pop(1);
        }
    }
}

/*
 * Returns an iterator to the ClientInfo whose thread ID matches tid.
 * If no match is found, the iterator equals client_info_vec.end().
 */
vector<ClientInfo>::iterator ClientManager::find_client (pthread_t tid) {
    return std::find_if(client_info_vec.begin(), client_info_vec.end(),
        [tid](ClientInfo& ci) {
            return pthread_equal(ci.get_tid(), tid);
        });
}

/*
 * Cleanup handler for client_thread().
 * - Cancels and joins the two worker threads stored in rd_ptr->tid_arr.
 * - Releases per-client resources (mutex, RtspData, RD).
 * - Removes the current client entry from client_info_vec.
 */
void ClientManager::remove (void * arg) {
    RD * rd_ptr = (RD *)arg;
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
        ret = pthread_join(tid, NULL);
        if (ret == ESRCH) {
            fprintf(stderr, "Warning: thread %lx does not exist (join returned ESRCH)\n", tid);
        }
        else if (ret != 0) {
            fprintf(stderr, "Error: %lx pthread_join failed\n", tid);
        }
    }
    // Free per-client resources
    pthread_mutex_destroy(&rd_ptr->rtd_ptr->m_cam_rq);
    free (rd_ptr->rtd_ptr);
    free (arg);
    printf("REMOVE: spawned %lx\t%lx\n", tid_arr[0], tid_arr[1]);
    // Erase this client’s record from client_info_vec
    pthread_mutex_lock(&(cm_ptr->m_client_info_vec));
        auto it = cm_ptr->find_client(pthread_self());
        if (it == cm_ptr->client_info_vec.end()) {
            fprintf(stderr, "Error: Find(remove) failed\n");
            pthread_mutex_unlock(&(cm_ptr->m_client_info_vec));
            return;
        }
        fprintf(stderr, "REMOVE: %lx\n", it->get_tid());
        close (it->get_sock_fd());
        cm_ptr->client_info_vec.erase(it);
        pthread_cond_broadcast(&cm_ptr->cond_all_sent);      // Signal to cond_updated in check_map_update()
        if (cm_ptr->client_info_vec.empty() == true) {
            pthread_cond_signal(&cm_ptr->cond_clear);
        }
    pthread_mutex_unlock(&(cm_ptr->m_client_info_vec));
}

/*
 * Cancels every client thread, waits until client_info_vec is empty,
 * then destroys the associated mutex(m_client_info_vec).
 */
void ClientManager::clear () {
    pthread_t tid;
    pthread_mutex_lock(&cm_ptr->m_client_info_vec);
        for (auto it : cm_ptr->client_info_vec) {
            tid = it.get_tid();
            if (pthread_cancel(tid) != 0) {
                fprintf(stderr, "Error: %lx pthread_cancel failure\n", tid);
            }
        }
        fprintf(stderr, "Waiting clear in client_manager...\n");
        while (cm_ptr->client_info_vec.empty() != true) {
            printf("Cond waiting...\n");
            pthread_cond_wait(&cm_ptr->cond_clear, &cm_ptr->m_client_info_vec);
        }
    pthread_mutex_unlock(&cm_ptr->m_client_info_vec);
    pthread_mutex_destroy(&cm_ptr->m_client_info_vec);
    pthread_mutex_destroy(&cm_ptr->m_updated);
    fprintf(stderr, "Clear in client_manager\n");
}

void * ClientManager::send_mapdata (void * arg) {
    while(1) {
        sleep(1);
    }
}

//To Do
void end_streaming(void * arg){
    pthread_t tid = *(pthread_t *)arg;
    RED * red_ptr = (RED *)arg;
    if(red_ptr -> init_flag == true){
        pthread_cancel(red_ptr -> tid);
        pthread_join(red_ptr -> tid,NULL);
    }
}

void * ClientManager::rtsp (void * arg) {
    /* DO NOT CHANGE */
        // Unblock SIGUSR1
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, SIGUSR1);
        pthread_sigmask(SIG_BLOCK, &set, NULL);

        // Unpackage RTD
        RTD * rtd_ptr = (RTD *)arg;
        int * cam_rq_ptr = &(rtd_ptr->cam_rq);
        pthread_mutex_t * m_cam_rq_ptr = &(rtd_ptr->m_cam_rq);
    /* DO NOT CHANGE */

    
    int prev_cam_id;
    pthread_t tid, prev_tid;
    StreamingModule streaming_module;
    
    RED red;
    red.init_flag = false;
    red.tid = prev_tid;

    SMD smd;
    smd.cam_id_ = -1;

    //pthread_create(&tid, NULL, &ClientIF::run, (void *) &cam_id);
    pthread_cleanup_push(end_streaming, (void *)&red);

    /* Replace : Authentication and Cam run */
    while (1) {
        struct timespec timeout = {0, 0};  // Non-blocking
        int sig;
        int ret = sigtimedwait(&set, nullptr, &timeout);

        if (ret == -1) {
            if (errno == EAGAIN) {
            }
            else {
                perror("Error: sigtimedwait failed");
            }
        }
        else if (ret == SIGUSR1) {
            cout << "[SIGUSR1 Received]\n";
            prev_cam_id = smd.cam_id_;
            prev_tid = tid;

            pthread_mutex_lock(m_cam_rq_ptr);
                pthread_cleanup_push(unlock_mutex, m_cam_rq_ptr);
                printf("CAM_RQ[%lx]: %d\n", pthread_self(), *cam_rq_ptr);
                smd.cam_id_ = *cam_rq_ptr;
            // pthread_mutex_unlock(m_cam_rq_ptr);
            pthread_cleanup_pop(1);
            
            cout << "[DEBUG] prev_cam_id : " << prev_cam_id << endl;
            cout << "[DEBUG] smd.cam_id_ : " << smd.cam_id_ << endl;
            if((prev_cam_id != -1) && (prev_cam_id != smd.cam_id_)){
                pthread_cancel(prev_tid);
                cout << "[DEBUG] TEST\n";
                pthread_join(prev_tid, NULL);
                cout << "[DEBUG] TEST2\n";
                pthread_create(&tid, NULL, streaming_module.run, (void *) &smd);

                if(red.init_flag == false){
                    red.init_flag = true;
                }
            }
            
        }
    }
    pthread_cleanup_pop(1);
    /* Replace */
    return nullptr;
}

/*
 * Per-client main thread.
 * ─ Allocates a per-client RD structure (each RD owns an RTD).
 * ─ Creates two worker threads:
 *     tid_arr[0] – send_mapdata()  // sends map data
 *     tid_arr[1] – rtsp()          // RTSP streaming
 * ─ Monitors client messages: logout and camera request(cam_rq)
 */
void * ClientManager::client_thread (void * arg) {
    int clnt_sock = *((int *)arg);
    free (arg);
    printf("PUSH: %d\t%lx\n", clnt_sock, pthread_self());
    // Make data that rtsp() and remove() need
    RTD * rtd_ptr = (RTD *)malloc(sizeof(RTD));
    rtd_ptr->cam_rq = clnt_sock;
    pthread_mutex_init(&rtd_ptr->m_cam_rq, NULL);
    RD * rd_ptr = (RD *)malloc(sizeof(RD));
    rd_ptr->rtd_ptr = rtd_ptr;
    // Push cleanup function
    pthread_cleanup_push(remove, (void *)rd_ptr);
    // Check id and pw
    if (check_idpw(clnt_sock) == 1) {
        fprintf(stderr, "Error: check_idpw failed\n");
        pthread_exit(NULL);
    }
    // Creates two worker threads
    if (pthread_create(&rd_ptr->tid_arr[0], NULL, send_mapdata, NULL) != 0) {
        fprintf(stderr, "Error: %d's pthread_create of send_mapdata failed\n", clnt_sock);
        pthread_exit(NULL);
    }
    if (pthread_create(&rd_ptr->tid_arr[1], NULL, rtsp, (void *)rtd_ptr) != 0) {
        fprintf(stderr, "Error: %d's pthread_create of rtsp failed\n", clnt_sock);
        pthread_exit(NULL);
    }
    printf("%d spawned tid_arr[0]: %lx\n", clnt_sock, rd_ptr->tid_arr[0]);
    printf("%d spawned tid_arr[1]: %lx\n", clnt_sock, rd_ptr->tid_arr[1]);
    // Receive(Detect) client messages: logout and camera request(cam_rq)
    if (recv_msg(clnt_sock, &rtd_ptr->cam_rq, rd_ptr->tid_arr, &rtd_ptr->m_cam_rq) == 1) {
        fprintf(stderr, "Error: recv_msg failed\n");
    }
    pthread_cleanup_pop(1);
    return nullptr;
}

void * ClientManager::check_map_update (void * arg) {
    return nullptr;
}


void ClientManager::set_cm (ClientManager * ptr) {
    cm_ptr = ptr;
}
