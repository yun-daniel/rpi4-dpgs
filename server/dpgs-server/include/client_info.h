#ifndef CLIENT_INFO_H
#define CLIENT_INFO_H

#include <pthread.h>

class ClientInfo
{
private:
    bool sent_map_flag;
    pthread_t tid;

public:
    ClientInfo ();
    bool get_sent_map_flag ();
    pthread_t get_tid ();

    int set_sent_map_flag (bool flag);
    int set_tid (pthread_t tid);
};

#endif  // CLIENT_INFO_H