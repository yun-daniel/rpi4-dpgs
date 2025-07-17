#include "connection_manager.h"

typedef struct RtspData {
    int cam_rq;                 // Cam number that received from client
    pthread_mutex_t m_cam_rq;   // Mutex for cam_rq
} RTD;

typedef struct ClientThreadArgs {
    int clnt_sock;
    ClientManager * clnt_mgr_ptr;
    ConnectionManager * conn_mgr_ptr;
} CTA;

typedef struct RemoveData {
    // StreamingModule * sm_ptr;
    RTD * rtd_ptr;
    SFA * sfa_ptr;
    pthread_t tid_arr[2];       // tid_arr[2] in client thread
                                // tid_arr[0] : SEND MAP
                                // tid_arr[1] : STREAMING
} RD;

int ConnectionManager::initialize () {

    port = 9999;

    client_info_vec_mutex = PTHREAD_MUTEX_INITIALIZER;
    empty_cv = PTHREAD_COND_INITIALIZER;

    // Setting detach option of pthread
    if (pthread_attr_init(&attr) != 0) {
        fprintf(stderr, "pthread_attr_init failure\n");
    }
    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
        fprintf(stderr, "pthread_attr_setdetachstate failure\n");
    }

    // Make new socket to listen and connect with port
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

int ConnectionManager::stop () {
    
    // Cancels every client thread, waits until client_info_vec is empty
    pthread_t tid;
    pthread_mutex_lock(&client_info_vec_mutex);
        for (auto it : client_info_vec) {
            tid = it.get_tid();
            if (pthread_cancel(tid) != 0) {
                fprintf(stderr, "Error: %lx pthread_cancel failure\n", tid);
            }
        }
        fprintf(stderr, "Waiting detach(remove) for stop in connection manager...\n");

        while (client_info_vec.empty() != true) {
            printf("Cond waiting...\n");
            pthread_cond_wait(&empty_cv, &client_info_vec_mutex);
        }
    pthread_mutex_unlock(&client_info_vec_mutex);

    printf("Cond waiting done..\n");

    clear();

    return 0;
}

int ConnectionManager::clear () {
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&client_info_vec_mutex);
    pthread_cond_destroy(&empty_cv);
    
    return 0;
}

vector<ClientInfo>::iterator ConnectionManager::find_client (pthread_t tid) {
    return std::find_if(client_info_vec.begin(), client_info_vec.end(),
        [tid](ClientInfo& ci) {
            return pthread_equal(ci.get_tid(), tid);
        });
}

void * ConnectionManager::run (void * arg) {

    SFA * sfa_ptr = (SFA *)arg;  
    ClientManager * clnt_mgr_ptr = sfa_ptr->clnt_mgr_ptr;
    ConnectionManager * conn_mgr_ptr = sfa_ptr->conn_mgr_ptr;

    int clnt_sock;
    pthread_t tid;
    CTA * cta_ptr;
    ClientInfo ci;

    while (1) {
        if ((clnt_sock = accept(conn_mgr_ptr->listen_fd, (struct sockaddr *) &conn_mgr_ptr->address, (socklen_t*)&conn_mgr_ptr->addrlen)) < 0) {
            perror("Error: accept"); 
            continue;
        }

        printf("Connected clnt_sock_ptr : %d\n", clnt_sock);

        cta_ptr = (CTA *)malloc(sizeof(CTA));
        cta_ptr->clnt_sock = clnt_sock;
        cta_ptr->clnt_mgr_ptr = clnt_mgr_ptr;
        cta_ptr->conn_mgr_ptr = conn_mgr_ptr;
        if (pthread_create(&tid, &conn_mgr_ptr->attr, client_thread, (void *)cta_ptr) != 0) {
            fprintf(stderr, "Error: %d's pthread_create of client_thread failed\n", clnt_sock);
            free (cta_ptr);
        }
        else {
            ci.set_sent_map_flag(false);
            ci.set_sock_fd(clnt_sock);
            ci.set_tid(tid);
            pthread_mutex_lock(&conn_mgr_ptr->client_info_vec_mutex);
                conn_mgr_ptr->client_info_vec.push_back(ci);
            pthread_mutex_unlock(&conn_mgr_ptr->client_info_vec_mutex);
        }
    }

    return 0;
}

/* Static Function */
void * ConnectionManager::client_thread (void * arg) {
    CTA * cta_ptr = (CTA *)arg;
    int clnt_sock = cta_ptr->clnt_sock;
    ClientManager * clnt_mgr_ptr = cta_ptr->clnt_mgr_ptr;
    ConnectionManager * conn_mgr_ptr = cta_ptr->conn_mgr_ptr; 
    free (arg);

    printf("PUSH: %d\t%lx\n", clnt_sock, pthread_self());

    // Make data that remove() needs
    RTD * rtd_ptr = (RTD *)malloc(sizeof(RTD));
    rtd_ptr->cam_rq = clnt_sock;
    pthread_mutex_init(&rtd_ptr->m_cam_rq, NULL);
    SFA * sfa_ptr = (SFA *)malloc(sizeof(SFA));
    sfa_ptr->clnt_mgr_ptr = clnt_mgr_ptr;
    sfa_ptr->conn_mgr_ptr = conn_mgr_ptr;
    RD * rd_ptr = (RD *)malloc(sizeof(RD));
    rd_ptr->rtd_ptr = rtd_ptr;
    rd_ptr->sfa_ptr = sfa_ptr;

    // Push cleanup function
    pthread_cleanup_push(remove, (void *)rd_ptr);

    // Check id and pw
    if (check_idpw(clnt_sock) == 1) {
        fprintf(stderr, "Error: check_idpw failed\n");
        pthread_exit(NULL); 
    }

    // Creates two worker threads
    if (pthread_create(&rd_ptr->tid_arr[0], NULL, send_mapdata, (void *)sfa_ptr) != 0) {
        fprintf(stderr, "Error: %d's pthread_create of send_mapdata failed\n", clnt_sock);
        pthread_exit(NULL);
    }
    if (pthread_create(&rd_ptr->tid_arr[1], NULL, streaming, (void *)rtd_ptr) != 0) {
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

        ret = pthread_join(tid, NULL);
        if (ret == ESRCH) {
            fprintf(stderr, "Warning: thread %lx does not exist (join returned ESRCH)\n", tid);
        }
        else if (ret != 0) {
            fprintf(stderr, "Error: %lx pthread_join failed\n", tid);
        }
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
        conn_mgr_ptr->client_info_vec.erase(it);

        // pthread_cond_broadcast(&conn_mgr_ptr->cond_all_sent);      // Signal to cond_updated in check_map_update()

        if (conn_mgr_ptr->client_info_vec.empty() == true) {
            pthread_cond_signal(&conn_mgr_ptr->empty_cv);
        }
    pthread_mutex_unlock(&conn_mgr_ptr->client_info_vec_mutex);

    // Free per-client resources
    pthread_mutex_destroy(&rd_ptr->rtd_ptr->m_cam_rq);
    free (rd_ptr->rtd_ptr);
    free (rd_ptr->sfa_ptr);
    free (arg);
}

/* Static Function */
void * ConnectionManager::send_mapdata (void * arg) {
    while (1) {
        sleep(1);
    }

    return nullptr;
}

/* Static Function */
void * ConnectionManager::streaming (void * arg) {
    /* DO NOT CHANGE */
        // Block SIGUSR1
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, SIGUSR1);
        pthread_sigmask(SIG_BLOCK, &set, NULL);

        // Unpackage RTD
        RTD * rtd_ptr = (RTD *)arg;
        int * cam_rq_ptr = &(rtd_ptr->cam_rq);
        pthread_mutex_t * m_cam_rq_ptr = &(rtd_ptr->m_cam_rq);
    /* DO NOT CHANGE */  

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
            printf("SIGUSR1 Received\n");
            
            pthread_mutex_lock(m_cam_rq_ptr);
                printf("CAM_RQ[%lx]: %d\n", pthread_self(), *cam_rq_ptr);
            pthread_mutex_unlock(m_cam_rq_ptr);

        }
    }
    /* Replace */

    return nullptr;
}