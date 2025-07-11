#include "client_manager.h"

#include <cstdio>

void * func (void * arg) {
    ClientManager * cm_ptr = (ClientManager *)arg;
    ClientManager::set_cm(cm_ptr);
    cm_ptr->connect_client();

    return nullptr;
}

int main (void) {
    /* Setting Signal */
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, SIGUSR1);
        pthread_sigmask(SIG_BLOCK, &set, NULL);
    /* Setting Signal */

    ClientManager cm(9090);

    pthread_t tid;
    pthread_create(&tid, NULL, func, (void *)&cm);

    int input;
    scanf("%d", &input);
    if (input > 0) {
        pthread_cancel(tid);
        ClientManager::clear();
    }

    pthread_join(tid, NULL);
    printf("bye\n");

}