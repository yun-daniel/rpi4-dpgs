#include "classifier.h"
#include <deque>

const float SLOT_THRESHOLD_RATIO    = 0.4;
const int   MAX_RATIOS              = 5;


static float computeOverlapArea(const std::vector<cv::Point>& poly, const cv::Rect& box) {
    cv::Rect bounding = cv::boundingRect(poly) | box;
    cv::Size canvasSize(bounding.x + bounding.width + 10, bounding.y + bounding.height + 10);

    cv::Mat mask1 = cv::Mat::zeros(canvasSize, CV_8UC1);
    cv::Mat mask2 = cv::Mat::zeros(canvasSize, CV_8UC1);

    std::vector<std::vector<cv::Point>> polys = { poly };
    cv::fillPoly(mask1, polys, cv::Scalar(255));

    cv::rectangle(mask2, box, cv::Scalar(255), cv::FILLED);

    cv::Mat intersection;
    cv::bitwise_and(mask1, mask2, intersection);

    float intersectionArea = static_cast<float>(cv::countNonZero(intersection));

    return intersectionArea;
}


ParkingStatusClassifier::ParkingStatusClassifier(MapManager& _mgr)
    : mgr(_mgr), map(_mgr.getMap()) {

    for (const auto& slot : map.slots) {
        slot_data[slot.slot_id] = SlotInfo();
    }
}


void ParkingStatusClassifier::updateState(int slot_id, SlotInfo& info) {
    float max_ratio = *std::max_element(info.ratios.begin(), info.ratios.end());
    std::string curr_state;

    for (auto& slot : map.slots) {
        if (slot_id == slot.slot_id) {
            curr_state = slot.state;
        }
    }
    

    if ((max_ratio > 0.5f) && (curr_state == "vacant")) {
            mgr.update_slot(slot_id, "occupied");
    }
    else if ((max_ratio <= 0.5f) && (curr_state == "occupied")) {
            mgr.update_slot(slot_id, "vacant");
    }
}




void ParkingStatusClassifier::update(const int slot_id, float ratio) {

    auto it = slot_data.find(slot_id);
    if (it == slot_data.end()) {
        std::cerr << "[AI][CLSF] Warning: Invalid update slot id\n";
    }
    auto& info = it->second;

    if (info.ratios.size() >= MAX_RATIOS)
        info.ratios.pop_front();
    info.ratios.push_back(ratio);

    updateState(slot_id, info);

}


void ParkingStatusClassifier::classify(const std::vector<Detection>& detections) {

    for (const auto& slot : map.slots) {
        float max_area = 0.0f;
        float ratio = 0.0f;
        float slot_area = std::fabs(cv::contourArea(slot.poly));

        for (const auto& detection : detections) {
            float area = computeOverlapArea(slot.poly, detection.box);
            if (area > max_area) {
                max_area = area;
            }
        }
        ratio = max_area / slot_area;

        update(slot.slot_id, ratio);


        std::cout << "[AI][CLSF] Slot " << slot.slot_id
            << ": " << max_area << ", ratio: " << ratio
            << ", " << "state: " << slot.state << "\n";
    }
}
