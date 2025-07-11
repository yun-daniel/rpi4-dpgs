#include "remote_led_dp.h"

#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>


RemoteLedDP::RemoteLedDP(MapManager& _map_mgr)
    : map_mgr(_map_mgr) {

    target_ip = TARGET_IP;
    target_port = TARGET_PORT;

}

RemoteLedDP::~RemoteLedDP() {
    if (sock_fd >= 0) close(sock_fd);
}


bool RemoteLedDP::initialize() {
    std::cout << "[RLDP] Start to initialize...\n";

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("[RLDP] Error: Failed to create socket");
        return false;
    }

    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(target_port);
    inet_pton(AF_INET, target_ip.c_str(), &target_addr.sin_addr);


    std::cout << "[RLDP] Success: Remote LED Display initialized\n";
    return true;
}


void RemoteLedDP::run() {
    std::cout << "[RLDP] Start Remote LED Display\n";


    is_running = true;

    while(is_running) {

        pthread_mutex_lock(map_mgr.get_mutex_dev());
        while ((!*(map_mgr.get_flag_ptr_dev())) && is_running ) {
            std::cout << "[TEST] RLDP Wait...\n";
            pthread_cond_wait(map_mgr.get_cv_dev(), map_mgr.get_mutex_dev());
        }
        *map_mgr.get_flag_ptr_dev() = false;
        pthread_mutex_unlock(map_mgr.get_mutex_dev());

        std::cout << "[RLDP] Updated Map Sending...\n";
        send_map();

    }


    std::cout << "[RLDP] Remote LED Display Terminated\n";
}


void RemoteLedDP::stop() {
    std::cout << "[RLDP] Remote LED Display Terminating...\n";

    is_running = false;
    pthread_mutex_lock(map_mgr.get_mutex_dev());
    pthread_cond_signal(map_mgr.get_cv_dev());
    pthread_mutex_unlock(map_mgr.get_mutex_dev());

    if (sock_fd >= 0) close(sock_fd);
}


void RemoteLedDP::send_map() {
    const SharedParkingLotMap& map = map_mgr.getMap();
    for (int i=0; i<map.total_slots; ++i) {
        int slot_id = map.slots[i].slot_id;
        SlotState state = map.slots[i].state;

        send_packet(slot_id, state);
    }
}


void RemoteLedDP::send_packet(const int slot_id, const int state) {
    std::string cmd = std::to_string(slot_id) + " " + std::to_string(state);
//    sendto(sock_fd, cmd.c_str(), cmd.size(), 0, (struct sockaddr*)&target_addr, sizeof(target_addr));
    std::cout << "[RLDP] send cmd: " << cmd << "\n";
}
