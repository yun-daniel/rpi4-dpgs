#include <iostream>
#include <atomic>
#include <csignal>
#include <pthread.h>

#include "dpgs_server.h"


// ===  Server SW Signal Handle ===
DPGSServer*         g_sys = nullptr;
std::atomic<bool>   stopping{false};

void signal_handler(int signo) {
    if (!stopping.exchange(true)) {
        std::cout << "[MAIN] Received Sig: " << signo << "\n";
        if (g_sys) {
            g_sys->stop();
        }
    }
    else {
        std::cout << "[MAIN] Signal already handled, ignoring.\n";
    }
}

void config_signal_mask() {
    sigset_t sigset;
    sigfillset(&sigset);
    sigdelset(&sigset, SIGINT);
    sigdelset(&sigset, SIGTERM);
    pthread_sigmask(SIG_SETMASK, &sigset, nullptr);
}

void register_signal_handlers() {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
}
// =======


int main(void) {

    std::cout << "******* Dynamic Parking Guidance System *******\n";

    config_signal_mask();
    register_signal_handlers();

    DPGSServer sys;
    if (!sys.initialize()) {
        std::cerr << "[MAIN] Error: Failed to initialize system\n";
        return 1;
    }
    g_sys = &sys;

    sys.start();

    sys.stop();

//    sys.clear();

    std::cout << "***********************************************\n";
    return 0;
}
