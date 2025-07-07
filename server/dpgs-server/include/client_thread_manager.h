#ifndef CLIENT_THREAD_MANAGER_H
#define CLIENT_THREAD_MANAGER_H

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <algorithm>

#include <unistd.h>
#include <errno.h>
#include <pthread.h>

using namespace std;

class ClientThreadManager 
{
private:
    // listen_fd
    vector<pthread_t> client_tid_vec;    // Subthreads spawned for each connected client      

    /* For signal_mapdata() */
    int mapdata;
    int sent_counter;                   
    int temp_total_clients;  
    bool mapdata_ready;
    pthread_mutex_t cond_mutex;
    pthread_mutex_t m1; 
    pthread_cond_t cond;


public:
    ClientThreadManager();
    int initialize();
    void connect_client();
    void remove(void *arg);
    int clear();
    int siganl_mapdata();           

    /* For signal mapdata() */
    void * send_mapdata(void *arg);

};

#endif  // CLIENT_THREAD_MANAGER_H
