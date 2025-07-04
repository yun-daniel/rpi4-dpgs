#include <iostream>
#include <cstdlib>

#include <unistd.h>

#include "core_thread_manager.h"

using namespace std;

void * hello (void *arg) {
    printf("hello dabbee\n");

    return nullptr;
}

/*
 * Create one thread for each task:
 *   1. Client connection manager — sends map data
 *   2. Client connection manager — receives ID/password
 *   3. Camera interface
 *   4. ISP processing
 *   5. Device manager
 *
 * Returns 0 on success, 1 on failure.
 */
int setting_threads (CoreThreadManager * ctm_ptr) {

    // Thread 1 : client connection manager (send map data)
    if (ctm_ptr->add_thread(hello) == 1) {
        return 1;
    }
    
    return 0;
}

int main (void) {
    
    CoreThreadManager ctm;

    if (setting_threads(&ctm) == 1) {
        fprintf(stderr, "setting_threads() failed\n");
        exit(EXIT_FAILURE);
    }

    // sleep(1);

    ctm.clear();

    return 0;
}