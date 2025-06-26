#include "ai_engine.h"

#include <fstream>


cv::Mat frame_sampling() {
    cv::Mat image = cv::imread("config/parking.jpg");

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


cv::Mat letterbox(const cv::Mat &src) {
    int col = src.cols;
    int row = src.rows;
    int max = std::max(col, row);

    cv::Mat dst = cv::Mat::zeros(max, max, CV_8UC3);
    src.copyTo(dst(cv::Rect(0, 0, col, row)));

    return dst;
}


void detector(cv::Mat &_frame, cv::dnn::Net &net, std::vector<Detection> &output, const std::vector<std::string> &class_name) {
    cv::Mat blob;
    std::vector<cv::Mat> outputs;

    auto frame = letterbox(_frame);

    blob = cv::dnn::blobFromImage(frame, 1.0/255.0, cv::Size(640, 640), cv::Scalar(), true, false);

    net.setInput(blob);
    net.forward(outputs, net.getUnconnectedOutLayersNames());
}


void run_ai_engine() {
    // Declare Variables
    std::vector<std::string>    class_list;
    cv::dnn::Net                net;
    cv::Mat                     frame;

    // Initialize
    init_dnn(class_list, net);


    // Run
//    while (true) {
        frame = frame_sampling();
        if (frame.empty()) {
            std::cout << "[DEBUG] No image\n";
//            break;
        }

        cv::imshow("Frame", frame);


//    }


}
