#include <iostream>

#include "system_controller.h"


int main(void) {

    std::cout << "******* Dynamic Parking Guidance System *******\n";


    SystemController sys;

    if (!sys.initialize()) {
        std::cerr << "[MAIN] Error: Failed to initialize system\n";
        return 1;
    }

    sys.start();

    sys.stop();


    std::cout << "***********************************************\n";
    return 0;
}
