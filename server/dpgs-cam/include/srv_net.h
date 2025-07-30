#ifndef __SERVER_NETWORK_H__
#define __SERVER_NETWORK_H__

#include "str_frame_buffer.h"
#include "srv_stream.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <thread>


class SrvNet {
 public:
    SrvNet(StrFrameBuffer& _fb);
    ~SrvNet();

    bool initialize();
    void run();
    void stop();

 private:
    bool is_running = false;

    int listen_fd, port;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    std::unique_ptr<SrvStream>  srv_stream;

    std::thread thread_srv_str;

    void clear();


    // External Interface
    StrFrameBuffer& fb;   


};




#endif // __SERVER_NETWORK_H__
