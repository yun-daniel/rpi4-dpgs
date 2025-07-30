#include "srv_net.h"


SrvNet::SrvNet(StrFrameBuffer& _fb)
    : fb(_fb) {
}

SrvNet::~SrvNet() {
}


bool SrvNet::initialize() {
    std::cout << "[SRV_NET] Start to initialize...\n";

    std::cout << "[SRV_NET] Initialize Server Streaming Module\n";
    srv_stream = std::make_unique<SrvStream>(fb);
    if (!srv_stream->initialize()) {
        std::cerr << "[SRV_NET] Error: Failed to initialize Server Streaming Module\n";
        return false;
    }


    // Network Init
    std::cout << "[SRV_NET] Initialize Network\n";
    port = 9999;
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == 0) {
        std::cerr << "[SRV_NET] Error: Failed to create socket\n";
        return false;
    }

    int optval = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        std::cerr << "[SRV_NET] Error: setsockopt\n";
        return false;
    }

    memset(&address, '0', sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    if (bind(listen_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "[SRV_NET] Error: bind\n";
        return false;
    }
    if (listen(listen_fd, 16) < 0) {
        std::cerr << "[SRV_NET] Error: listen\n";
        return false;
    }


    std::cout << "[SRV_NET] Success: Server Network Initialized\n";
    return true;
}


void SrvNet::run() {
    std::cout << "[SRV_NET] Run Server Network Module\n";

    int clnt_sock;

    is_running = true;
    while (is_running) {
        if ((clnt_sock = accept(listen_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            if (!is_running) break;
            std::cerr << "[SRV_NET] Error: accept\n";
            continue;
        }
        std::cout << "[SRV_NET] Connected Main Server: clnt_sock " << clnt_sock << "\n";

        thread_srv_str = std::thread([this](){
            srv_stream->run();
        });

    }

    std::cout << "[SRV_NET] Server Network Module Terminated\n";
}


void SrvNet::stop() {
    std::cout << "[SRV_NET] Server Network Module Terminating...\n";

    srv_stream->stop();

    clear();

    is_running = false;
    int dummy = socket(AF_INET, SOCK_STREAM, 0);
    connect(dummy, (struct sockaddr*)&address, sizeof(address));
    close(dummy);
    close(listen_fd);

}


void SrvNet::clear() {
    std::cout << "[SRV_NET] clear: Cleanning...\n";

    if (thread_srv_str.joinable()) {
        std::cout << "[SRV_NET] Joining thread_srv_str\n";
        thread_srv_str.join();
    }


    std::cout << "[SRV_NET] clear: Cleanning Success\n";
}
