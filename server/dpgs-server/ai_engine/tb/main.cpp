#include "ai_engine.h"
#include "map_manager.h"
#include "frame_buffer.h"
#include "vpe_dummy.h"

#include <iostream>
#include <thread>
#include <unistd.h>
#include <sys/wait.h>


int main(int argc, char **argv) {

    FrameBuffer fb("FB");
    if (!fb.initialize()) {
        std::cerr << "Failed to initialize FrameBuffer\n";
        return 1;
    }

    MapManager mgr("config/map.json");
    if (!mgr.initialize()) {
        std::cerr << "Failed to initialize MapManager\n";
        return 1;
    }


//    VpeDummy vpe("tb/test_640x360.mp4", fb);
    VpeDummy vpe("tb/test_a.mp4", fb);
    std::thread vpe_thread([&]() {
        vpe.start();
    });

    pid_t pid = fork();
    if (pid == 0) {
        AIEngine engine(fb, mgr);
        engine.run();
        return 0;
    }


    vpe_thread.join();



    // After AI Engine Process Terminated
    fb.destroyShm();


//    std::cout << "[DEBUG] Run Idle\n";
//    while (1) {
//    }

    return 0;
}
