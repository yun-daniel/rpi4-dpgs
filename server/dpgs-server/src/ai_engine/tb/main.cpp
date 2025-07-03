#include "ai_engine.h"
#include "map_manager.h"
#include "frame_buffer.h"

#include <iostream>


int main(int argc, char **argv) {

//    FrameBuffer fb("FB");
//    if (!fb.initialize()) {
//        std::cerr << "Failed to initialize FrameBuffer\n";
//        return 1;
//    }

    run_ai_engine();

    std::cout << "[DEBUG] Run Idle\n";
    while (1) {
    }


    // MapManager TEST Session ----
//    MapManager mgr("config/map.json");
//
//    mgr.insert_slot(4, "occupied", cv::Rect(400, 500, 90, 110));
//    mgr.printMap();
//
//    mgr.update_slot(4, "vacant");
//    mgr.printMap();
//
//    mgr.update_slot(99, "occupied");
//
//    mgr.delete_slot(4);
//
//    std::cout << "\nUpdated Map\n";
//    mgr.printMap();
//
//    mgr.save();
//    std::cout << "Saved updated map.json\n";
    // --------------

    return 0;
}
