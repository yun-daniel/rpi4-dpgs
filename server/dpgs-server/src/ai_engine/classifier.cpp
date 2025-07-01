#include "classifier.h"


const float SLOT_THRESHOLD_RATIO = 0.4;


float computeOverlapArea(const std::vector<cv::Point>& poly, const cv::Rect& box) {
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


void classifier(const std::vector<Detection>& detections, MapManager& mgr) {
    const ParkingLotMap& map = mgr.getMap();

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

        if ((ratio > 0.5f) && (slot.state == "vacant")) {
            mgr.update_slot(slot.slot_id, "occupied");
        }
        else if ((ratio <= 0.5f) && (slot.state == "occupied")) {
            mgr.update_slot(slot.slot_id, "vacant");
        }

        std::cout << "[AI][CLSF] Slot " << slot.slot_id
            << ": " << max_area << ", ratio: " << ratio
            << ", " << "state: " << slot.state << "\n";
    }
}
