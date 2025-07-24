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


bool AIEngine::Impl::initialize() {
    std::cout << "[AI] Start to initialize...\n";

    init_dnn();

    std::cout << "[AI] Success: AI Engine initialized\n";
    return true;
}


void AIEngine::Impl::run() {
    std::cout << "[AI] Start AI Engine\n";

    const SharedParkingLotMap& map = mgr.getMap();
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
        classifier.classify(frame, detections);


        // [Debug Session]
//        overlay_slots(frame, map);
//        overlay_detection(frame, detections, class_list);
//        print_frame(frame);
//        cv::waitKey(1);
        // --------------

    }

    std::cout << "[AI] AI Engine Terminated\n";

}


void AIEngine::Impl::stop() {
    std::cout << "[AI] AI Engine Terminating...\n";

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
    fb.pop(image);

    return image;
}


void AIEngine::Impl::overlay_slots(cv::Mat& frame, const SharedParkingLotMap& map) {

    for (int i=0; i<map.total_slots; ++i) {
        const auto& slot = map.slots[i];

        const cv::Point* pts = slot.poly;
        int npts = 4;

        cv::Scalar color;
        switch (slot.state) {
            case EMPTY:
                color = colors[EMPTY]; break;
            case OCCUPIED:
                color = colors[OCCUPIED]; break;
            case EXITING:
                color = colors[EXITING]; break;
            default:
                color = colors[0]; break;
        }

        cv::polylines(frame, &pts, &npts, 1, true, color, 3);
    }
}


void AIEngine::Impl::print_frame(const cv::Mat& frame) {
    cv::imshow("output", frame);
}


// === AIEngine ===
AIEngine::AIEngine(FrameBuffer& _fb, MapManager& _mgr)
    : impl(new Impl(_fb, _mgr)) {
}

AIEngine::~AIEngine() {
    delete impl;
}

bool AIEngine::initialize() {
    return impl->initialize();
}

void AIEngine::run() {
    impl->run();
}

void AIEngine::stop() {
    impl->stop();
}


