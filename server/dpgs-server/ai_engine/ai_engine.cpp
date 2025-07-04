#include "ai_engine.h"
#include "ai_engine_internal.h"
#include "detector.h"
#include "classifier.h"
#include "map_manager.h"

#include <fstream>
#include <unistd.h>
#include <limits.h>


// === AIEngine::Impl ===
AIEngine::Impl::Impl(FrameBuffer& _fb, MapManager& _mgr)
    : fb(_fb), mgr(_mgr), is_running(false) {
}

AIEngine::Impl::~Impl() {
    stop();
}


void AIEngine::Impl::run() {

    init_dnn();
//    init_test_video();

    const ParkingLotMap& map = mgr.getMap();
    ParkingStatusClassifier classifier(mgr);

    is_running = true;

    while (is_running) {
        cv::Mat frame = frame_sampling();
        if (frame.empty()) {
            std::cerr << "[AI][ENGINE] Warning: frame sampling: No frame\n";
            continue;
        }

        std::vector<Detection> detections;
        detector(frame, net, detections, class_list);
        classifier.classify(detections);

        overlay_slots(frame, map.slots);
        overlay_detection(frame, detections, class_list);
        print_frame(frame);

        if (cv::waitKey(1) != -1) break;
    }

    test_video.release();
}


void AIEngine::Impl::stop() {
    is_running = false;
}


std::vector<std::string> AIEngine::Impl::load_class_list() {
    std::vector<std::string> class_list;

    std::ifstream ifs(CLASS_LIST_FILE);
    std::string line;
    while (getline(ifs, line)) {
        class_list.push_back(line);
    }

    return class_list;
}


void AIEngine::Impl::init_dnn() {
    std::cout << "[AI][ENGINE] Initialize DNN\n";

    class_list = load_class_list();

    net = cv::dnn::readNet(ONNX_FILE);
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

    std::cout << "[AI][ENGINE] Success: Initialized DNN\n";
}


cv::Mat AIEngine::Impl::frame_sampling() {
    cv::Mat image;

//    test_video.read(image);
    fb.pop(image);

    return image;
}


void AIEngine::Impl::overlay_slots(cv::Mat& frame, const std::vector<Slot>& slots) {

    for (const auto& slot : slots) {

        if (slot.poly.size() != 4) continue;

        const cv::Point* pts = slot.poly.data();
        int npts = static_cast<int>(slot.poly.size());

        const auto color = (slot.state == "occupied")? colors[4] : colors[1];
        cv::polylines(frame, &pts, &npts, 1, true, color, 3);
    }
}


void AIEngine::Impl::print_frame(const cv::Mat& frame) {
    cv::imshow("output", frame);
}


// Only for Test
bool AIEngine::Impl::init_test_video() {
    test_video.open("tb/test_a.mp4");
    if (!test_video.isOpened()) {
        char cwd[PATH_MAX];
        getcwd(cwd, sizeof(cwd));
        std::cerr << "[AI][ENGINE] Error: Fail to open test_video source, " << cwd << "\n";
        return false;
    }

    std::cout << "[AI][ENGINE] Success: test_video source opened\n";
    return true;
}



// === AIEngine ===
AIEngine::AIEngine(FrameBuffer& _fb, MapManager& _mgr)
    : impl(new Impl(_fb, _mgr)) {
}

AIEngine::~AIEngine() {
    delete impl;
}

void AIEngine::run() {
    impl->run();
}

void AIEngine::stop() {
    impl->stop();
}

