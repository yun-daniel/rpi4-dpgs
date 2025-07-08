#ifndef __SENDER_H__
#define __SENDER_H__

#include "slot.h"
#include <string>
#include <vector>
#include <netinet/in.h>

class Sender {
public:
    Sender(const std::string& json_path, const std::string& target_ip, int port = 8888);
    ~Sender();

    void run();

private:
    std::string json_path;
    int sock_fd;
    struct sockaddr_in target_addr;

    std::vector<Slot> parse_slots();
    std::string read_file();
    void send_all(const std::vector<Slot>& slots);
    void send_packet(const Slot& slot);
};

#endif
