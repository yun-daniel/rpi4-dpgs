#include "sender/sender.h"
#include <thread>

int main() {
    Sender sender("map.json", "192.168.0.54", 8888);

    std::thread sender_thread([&]() {
        sender.run();
    });

    sender_thread.join();
    return 0;
}
