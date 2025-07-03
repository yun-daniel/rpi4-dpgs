#include "ai_engine_internal.h"
#include "ai_engine.h"
#include "detector.h"
#include "classifier.h"
#include "map_manager.h"

#include <fstream>
#include <unistd.h>
#include <limits.h>


cv::VideoCapture capture;

bool init_test_video() {
    capture.open("tb/test_a.mp4");
    if (!capture.isOpened()) {
        char cwd[PATH_MAX];
        getcwd(cwd, sizeof(cwd));
        std::cerr << "Error: Fail to open video source, " << cwd << "\n";
        return false;
    }

    std::cout << "Video source opened\n";
    return true;
}


cv::Mat frame_sampling() {
    cv::Mat image;
    capture.read(image);

    return image;
}


std::vector<std::string> load_class_list() {
    std::vector<std::string> class_list;

    std::ifstream ifs("config/classes.txt");
    std::string line;
    while (getline(ifs, line)) {
        class_list.push_back(line);
    }

    return class_list;
}


void init_dnn(std::vector<std::string> &class_list, cv::dnn::Net &net) {
    std::cout << "Initialize DNN setup\n";

    class_list = load_class_list();

    auto result = cv::dnn::readNet("config/yolov5s.onnx");
    result.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    result.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

    net = result;

    std::cout << "Finish DNN setup\n";
}


void overlay_slots(cv::Mat& frame, const std::vector<Slot>& slots) {

    for (const auto& slot : slots) {

        if (slot.poly.size() != 4) return;

        const cv::Point* pts = slot.poly.data();
        int npts = static_cast<int>(slot.poly.size());

        const auto color = (slot.state == "occupied")? colors[4] : colors[1];
        cv::polylines(frame, &pts, &npts, 1, true, color, 3);
    }
}


void print_frame(const cv::Mat& frame) {
    cv::imshow("output", frame);
    if (cv::waitKey(1) != -1) {
        capture.release();
        std::cout << "Finished by user\n";
        return;
    }
}


void run_ai_engine() {
    // Declare
    std::vector<std::string>    class_list;
    cv::dnn::Net                net;
//    cv::Mat                     frame;
    std::vector<Detection>      outputs;
    MapManager                  mgr("config/map.json");
    const ParkingLotMap&        map = mgr.getMap();
    ParkingStatusClassifier     classifier(mgr);

    // Initialize
    if (init_test_video() == false) {
        std::cerr << "Error: Video Init Failed\n";
        return;
    }
    init_dnn(class_list, net);


    // Run
    while (true) {
        cv::Mat frame = frame_sampling();
        if (frame.empty()) {
            std::cout << "[DEBUG] No image\n";
            break;
        }

        detector(frame, net, outputs, class_list);

        classifier.classify(outputs);

        overlay_slots(frame, map.slots);
        overlay_detection(frame, outputs, class_list);
        print_frame(frame);



    }


}
