#include "client_manager.h"

#include <cstdio>

void setup_sig_handler() {
    struct sigaction sa;
    sa.sa_handler = ClientManager::signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);
}

void * func (void * arg) {
    ClientManager * cm_ptr = (ClientManager *)arg;
    ClientManager::set_instance(cm_ptr);
    cm_ptr->connect_client();

    return nullptr;
}

int main (void) {
    // Setting Signal
    // HAVE TO STUDY!!
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    setup_sig_handler();

    ClientManager cm(9090);

    pthread_t tid;
    pthread_create(&tid, NULL, func, (void *)&cm);

    int input;
    scanf("%d", &input);
    if (input > 0) {
        pthread_cancel(tid);
        ClientManager::clear(&cm);
    }

    pthread_join(tid, NULL);
    printf("bye\n");

}