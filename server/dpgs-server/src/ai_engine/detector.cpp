#include "ai_engine_internal.h"
#include "detector.h"

#include <fstream>


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

    // Debug Session - show blob
//    std::vector<cv::Mat> channels;
//    cv::dnn::imagesFromBlob(blob, channels);
//    cv::imshow("blob", channels[0]);
//    cv::waitKey(0);
    // ----

    net.setInput(blob);
    net.forward(outputs, net.getUnconnectedOutLayersNames());

    output = post_detector(frame, outputs, class_name);
    

    // Debug Session    
    for (int i=0; i<output.size(); ++i) {
        std::cout << "[Output " << i
            << "] " << "class_id: " << output[i].class_id
            << ", " << "confidence: " << output[i].confidence
            << ", " << "[" << output[i].box.x
            << ", " << output[i].box.y
            << ", " << output[i].box.width
            << ", " << output[i].box.height << "]\n";
    }
}


std::vector<Detection> post_detector(cv::Mat &frame, std::vector<cv::Mat> &outputs, const std::vector<std::string> &className) {
    std::vector<Detection> output;

    float x_factor = frame.cols / INPUT_WIDTH;
    float y_factor = frame.rows / INPUT_HEIGHT;

    float *data = (float *)outputs[0].data;

    const int dim = 85;
    const int rows = 25200;

    std::vector<int> class_ids;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    for (int i=0; i<rows; ++i) {
        float confidence = data[4];

        if (confidence >= CONFIDENCE_THRESHOLD) {
            float *classes_scores = data + 5;
            cv::Mat scores(1, className.size(), CV_32FC1, classes_scores);
            cv::Point class_id;
            double max_class_score;

            minMaxLoc(scores, 0, &max_class_score, 0, &class_id);
            if (max_class_score > SCORE_THRESHOLD) {
                confidences.push_back(confidence);
                class_ids.push_back(class_id.x);

                float _x = data[0];
                float _y = data[1];
                float _w = data[2];
                float _h = data[3];            

                int x = int((_x-0.5 * _w) * x_factor);
                int y = int((_y-0.5 * _h) * y_factor);
                int width = int(_w * x_factor);
                int height = int(_h * y_factor);

                boxes.push_back(cv::Rect(x, y, width, height));
            }
        }

        data += 85;
    }

    std::vector<int> nms_result;
    cv::dnn::NMSBoxes(boxes, confidences, SCORE_THRESHOLD, NMS_THRESHOLD, nms_result);
    for (int i=0; i<nms_result.size(); ++i) {
        int idx = nms_result[i];
        Detection result;
        result.class_id = class_ids[idx];
        result.confidence = confidences[idx];
        result.box = boxes[idx];
        output.push_back(result);
    }

    return output;
}


void overlay_detection(cv::Mat& frame, std::vector<Detection> &outputs, const std::vector<std::string>& class_list) {
    int detections = outputs.size();

    for (int i=0; i<detections; ++i) {
        auto detection = outputs[i];
        auto box = detection.box;
        auto class_id = detection.class_id;
        const auto color = colors[class_id % (colors.size()-1)];
        cv::rectangle(frame, box, color, 3);

        cv::rectangle(frame, cv::Point(box.x, box.y-20), cv::Point(box.x+box.width, box.y), color, cv::FILLED);
        cv::putText(frame, class_list[class_id].c_str(), cv::Point(box.x, box.y-5), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,0));
    }
}




