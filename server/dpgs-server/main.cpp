#include <cstdio>

#include <pthread.h>

#include "client_manager.h"

int main (void) {

    ClientManager cm;
    cm.initialize();

    pthread_t tid;
    pthread_create(&tid, NULL, ClientManager::run, (void *)&cm);

    int input;
    scanf("%d", &input);
    if (input > 0) {
        cm.stop();
        pthread_join(tid, NULL);
    }

    return 0;
}