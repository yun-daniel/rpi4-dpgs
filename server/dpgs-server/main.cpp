#include <cstdio>
#include <thread>

#include "client_manager.h"

int main (void) {

    ClientManager cm;
    cm.initialize();

    std::thread t([&cm]() {
        cm.run();
    });

    // cm, map_manager, vpe

    int input;
    scanf("%d", &input);
    if (input > 0) {
        cm.stop();
        t.join();
    }

    return 0;
}