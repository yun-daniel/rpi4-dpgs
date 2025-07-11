#ifndef __REMOTE_LED_DP_H__
#define __REMOTE_LED_DP_H__

#include "map_manager.h"

#include <netinet/in.h>

#define TARGET_IP   "192.168.0.49"
#define TARGET_PORT 8888


class RemoteLedDP {
 public:
    RemoteLedDP(MapManager& _map_mgr);
    ~RemoteLedDP();

    bool initialize();
    void run();
    void stop();

 private:
    bool    is_running = false;
    int     sock_fd = -1;
    std::string target_ip;
    int         target_port;
    struct sockaddr_in target_addr;

    void send_map();
    void send_packet(const int slot_id, const int state);

    // External Interface
    MapManager&             map_mgr;
//    SharedParkingLotMap*    map = nullptr;

};


#endif // __REMOTE_LED_DP_H__
