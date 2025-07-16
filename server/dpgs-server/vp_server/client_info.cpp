#include "client_info.h"

ClientInfo::ClientInfo () : sent_map_flag(false), sock_fd(-1) {

}

bool ClientInfo::get_sent_map_flag () {
    return sent_map_flag;
}

int ClientInfo::get_sock_fd () {
    return sock_fd;
}

pthread_t ClientInfo::get_tid () {
    return tid;
}

int ClientInfo::set_sent_map_flag (bool flag) {
    sent_map_flag = flag;

    return 0;
}

int ClientInfo::set_sock_fd (int fd) {
    sock_fd = fd;

    return 0;
}

int ClientInfo::set_tid (pthread_t _tid) {
    tid = _tid;

    return 0;
}