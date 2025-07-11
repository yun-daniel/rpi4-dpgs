#include "client_manager.h"

ClientManager* ClientManager::instance_ptr = nullptr;

typedef struct remove_data {
    ClientManager * cm_ptr;
    pthread_t * tid_arr;        // tid_arr[2] in client thread
                                // tid_arr[0] : MAP SEND
                                // tid_arr[1] : RTSP
} RD;

typedef struct client_thread_data {
    ClientManager * cm_ptr;
    int clnt_sock;
} CTD;

ClientManager::ClientManager (int _port) : port(9999) {
    port = _port;
    m_client_info_vec = PTHREAD_MUTEX_INITIALIZER;
    m_cam_rq = PTHREAD_MUTEX_INITIALIZER;

    cam_rq = 0;

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
    CTD * ctd_ptr;
    pthread_t tid;

    int i = 0;
    int a = 0;

    while (1) {
        pthread_testcancel();

        ctd_ptr = (CTD *)malloc(sizeof(CTD));
        ctd_ptr->cm_ptr = this;

        /*
        if ((ctd_ptr->clnt_sock = accept(listen_fd, (struct sockaddr *) &address, (socklen_t*)&addrlen)) < 0) {
            perror("Error: accept"); 
            free (ctd_ptr);
            continue;
        }

        printf("Connected clnt_sock_ptr : %d\n", ctd_ptr->clnt_sock);
        */

        ctd_ptr->clnt_sock = i++;
    
        // add tid;
        if (pthread_create(&tid, &attr, client_thread, (void *)ctd_ptr) != 0) {
            fprintf(stderr, "Error: %d's pthread_create of mapdata failed\n", ctd_ptr->clnt_sock);
            free (ctd_ptr);
        }
        else {
            ci.set_sent_map_flag(false);
            ci.set_tid(tid);
            pthread_mutex_lock(&m_client_info_vec);
                pthread_cleanup_push(unlock_mutex, (void *)&m_client_info_vec);
                client_info_vec.push_back(ci);
            pthread_cleanup_pop(1);
        }

        sleep(3);

        // a++;
        // if (a == 3) {
        //     break;
        // }
    }

    ClientManager::clear(ClientManager::instance_ptr);
}

/*
 * HAVE TO STUDY!! 
 */
vector<ClientInfo>::iterator ClientManager::find_client (pthread_t tid) {
    return std::find_if(client_info_vec.begin(), client_info_vec.end(),
        [tid](ClientInfo& ci) {
            return pthread_equal(ci.get_tid(), tid);
        });
}

/*
 * Cleanup handler for client_thread().
 * tid_vec[2] cancel + join, erase tid from client_tid_vec
 */
void ClientManager::remove (void * arg) {
    RD * rd_ptr = (RD *)arg;
    ClientManager * cm_ptr = rd_ptr->cm_ptr;
    pthread_t tid_arr[2];
    memcpy(tid_arr, rd_ptr->tid_arr, sizeof(pthread_t) * 2);
    free (rd_ptr->tid_arr);
    free (arg);

    // tid_arr[2] cancel + join
    for (pthread_t tid : tid_arr) {
        if (pthread_cancel(tid) != 0) {
            fprintf(stderr, "Error: %lx pthread_cancel failed\n", tid);
        }
        if (pthread_join(tid, NULL) != 0) {
            fprintf(stderr, "Error: %lx pthread_join failed\n", tid);
        }
    }

    printf("REMOVE: spawned %lx\t%lx\n", tid_arr[0], tid_arr[1]);
    
    // erase pthread_self() fron this->client_tid_vec
    pthread_mutex_lock(&(cm_ptr->m_client_info_vec));
        auto it = cm_ptr->find_client(pthread_self());
        if (it == cm_ptr->client_info_vec.end()) {
            fprintf(stderr, "Error: Find(remove) failed\n");
            pthread_mutex_unlock(&(cm_ptr->m_client_info_vec));
            return;
        }
        fprintf(stderr, "REMOVE: %lx\n", it->get_tid());
        cm_ptr->client_info_vec.erase(it);
    pthread_mutex_unlock(&(cm_ptr->m_client_info_vec));
}

/*
 * Delete all elements of client_tid_vec
 * and all cancel, flag and detach check
 */
void ClientManager::clear (void * arg) {
    ClientManager * cm_ptr = (ClientManager *)arg;
    pthread_t tid;

    pthread_mutex_lock(&(cm_ptr->m_client_info_vec));
        for (auto it : cm_ptr->client_info_vec) {
            tid = it.get_tid();
            if (pthread_cancel(tid) != 0) {
                fprintf(stderr, "Error: %lx pthread_cancel failure\n", tid);
            }
        }
        fprintf(stderr, "Waiting clear in client_manager...\n");
    pthread_mutex_unlock(&(cm_ptr->m_client_info_vec));

    // HAVE TO FIX : Mutex and Cond
    while (cm_ptr->client_info_vec.empty() != true) {
        usleep(10000);
    }
    
    fprintf(stderr, "Clear in client_manager\n");
}

void * send_mapdata (void * arg) {
    while(1) {
        sleep(1);
    }
}

void * rtsp (void * arg) {
    /* DO NOT CHANGE */
    // Unblock SIGUSR1
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);
    /* DO NOT CHANGE */

    /* Replace : Authentication and Cam run */
    while (1) {
        sleep(1);
    }
    /* Replace */
}

void * ClientManager::client_thread (void * arg) {
    CTD * ctd_ptr = (CTD *)arg;
    ClientManager * cm_ptr = ctd_ptr->cm_ptr;
    int clnt_sock = ctd_ptr->clnt_sock;
    free (arg);
    
    printf("PUSH: %d\t%lx\n", clnt_sock, pthread_self());

    pthread_t * tid_arr = (pthread_t *)malloc(sizeof(pthread_t) * 2);
    RD * rd_ptr = (RD *)malloc(sizeof(RD));
    rd_ptr->cm_ptr = cm_ptr;
    rd_ptr->tid_arr = tid_arr;

    pthread_cleanup_push(remove, (void *)rd_ptr);

    pthread_create(&tid_arr[0], NULL, send_mapdata, NULL);
    pthread_create(&tid_arr[1], NULL, rtsp, NULL);
    
    printf("%d spawned tid_arr[0]: %lx\n", clnt_sock, tid_arr[0]);
    printf("%d spawned tid_arr[1]: %lx\n", clnt_sock, tid_arr[1]);

    // detect logout, cam_rq
    /*
    if logout : pthread_exit(1)
    if cam_rq : pthread_kill(tid_arr[1])
    */
    int i = 0; 
    while (i != 3) {
        pthread_mutex_lock(&instance_ptr->m_cam_rq); 
            if (i == 2) {
                cm_ptr->cam_rq = 2;
            }
            else {
                cm_ptr->cam_rq = 1;
            }
        pthread_mutex_unlock(&instance_ptr->m_cam_rq); 
    
        pthread_kill(tid_arr[0], SIGUSR1);
        pthread_kill(tid_arr[1], SIGUSR1);
        i++;
        sleep(1);
    }

    pthread_join(tid_arr[0], NULL);
    pthread_join(tid_arr[1], NULL);

    pthread_cleanup_pop(1);

    return nullptr;
}

void * ClientManager::check_map_update (void * arg) {
    return nullptr;
}

void ClientManager::unlock_mutex (void * arg) {
    pthread_mutex_t * m = static_cast<pthread_mutex_t *>(arg);
    pthread_mutex_unlock(m);
}

void ClientManager::set_instance (ClientManager * ptr) {
    instance_ptr = ptr;
}

void ClientManager::signal_handler (int sig) {
    /* DO NOT CHANGE */
    int cam_rq_cp;
    if (sig == SIGUSR1 && instance_ptr != nullptr) {
        pthread_mutex_lock(&instance_ptr->m_cam_rq);
            cam_rq_cp = instance_ptr->cam_rq;
        pthread_mutex_unlock(&instance_ptr->m_cam_rq);
        /* DO NOT CHANGE */

        /* Replace : Convert cam using cam_rq_cp */
        fprintf(stderr, "SIGUSER1[%lx] >> SIGUSR1 received, cam_rq: %d\n", pthread_self(), cam_rq_cp);
        /* Replace */
    }
}
