#include "client_manager.h"

#include <cstdio>

void * func (void * arg) {
    ClientManager * cm_ptr = (ClientManager *)arg;
    cm_ptr->connect_client();

    return nullptr;
}

int main (void) {
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