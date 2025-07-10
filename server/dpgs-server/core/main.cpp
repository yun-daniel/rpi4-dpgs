#include <iostream>

#include "dpgs_server.h"


int main(void) {

    std::cout << "******* Dynamic Parking Guidance System *******\n";


    DPGSServer sys;

    if (!sys.initialize()) {
        std::cerr << "[MAIN] Error: Failed to initialize system\n";
        return 1;
    }

    sys.start();

    sys.stop();


    std::cout << "***********************************************\n";
    return 0;
}
