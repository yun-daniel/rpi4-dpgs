#include "classifier.h"
#include <deque>

const float SLOT_THRESHOLD_RATIO    = 0.5;
const int   HEADLIGHT_THRESHOLD     = 95;
const int   NUM_SLOT_INFO           = 20;
const int   OCCUPIED_THRESHOLD      = 20;
const int   EXITING_THRESHOLD       = 20;


// === Utility ===
static float compute_overlap_area(const std::vector<cv::Point>& poly, const cv::Rect& box) {
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

static float get_brightness(const cv::Mat& frame, const cv::Rect& box, cv::Rect& out_rect) {
    int h_baseline = box.height/2;

    cv::Rect lower_rect(box.x, box.y+h_baseline, box.width, h_baseline);
    lower_rect &= cv::Rect(0, 0, frame.cols, frame.rows);
    out_rect = lower_rect;

    if (lower_rect.width <= 0 || lower_rect.height <= 0) return -1.0f;

    cv::Mat roi = frame(lower_rect);
    cv::Scalar mean_bgr = cv::mean(roi);

    float luminance = 0.114f * mean_bgr[0] + 0.587f * mean_bgr[1] + 0.299f * mean_bgr[2];

    return luminance;
}
// =============


// === ParkingStatusClassifier ===
ParkingStatusClassifier::ParkingStatusClassifier(MapManager& _mgr)
    : mgr(_mgr), map(_mgr.getMap()) {

    for (int i=0; i<map.total_slots; ++i) {
        const auto& slot = map.slots[i];
        slot_data[slot.slot_id] = SlotInfo();
    }
}


void ParkingStatusClassifier::updateState(int slot_id, SlotInfo& info) {
    float max_ratio = *std::max_element(info.ratios.begin(), info.ratios.end());
    float max_bright = *std::max_element(info.brights.begin(), info.brights.end());
    SlotState curr_state;
    SlotState* prev_state = &info.prev_state;

    for (int i=0; i<map.total_slots; ++i) {
        const Slot& slot = map.slots[i];
        if (slot_id == slot.slot_id) {
            curr_state = slot.state;
        }
    }


    switch (curr_state) {
        case EMPTY:
            if (max_ratio <= SLOT_THRESHOLD_RATIO) {
//                mgr.update_slot(slot_id, EMPTY);
//                info.state_cnt = 0;
            }
            else {
                mgr.update_slot(slot_id, OCCUPIED);
            }
            break;
        case OCCUPIED:
            if (max_ratio <= SLOT_THRESHOLD_RATIO) {
                mgr.update_slot(slot_id, EMPTY);
                info.state_cnt = 0;
            }
            else {
                if (max_bright <= HEADLIGHT_THRESHOLD) {
                    info.state_cnt++;
                }
                else {
                    if (info.state_cnt >= OCCUPIED_THRESHOLD) {
                        info.state_cnt++;
                    }
                    else {
                        mgr.update_slot(slot_id, EXITING);
                        info.state_cnt = 0;
                    }
                }
            }
            break;
        case EXITING:
            if (max_ratio <= SLOT_THRESHOLD_RATIO) {
                mgr.update_slot(slot_id, EMPTY);
                info.state_cnt = 0;
            }
            else {
                if (max_bright <= HEADLIGHT_THRESHOLD) {
                    if (info.state_cnt >= EXITING_THRESHOLD) {
                        mgr.update_slot(slot_id, OCCUPIED);
                        info.state_cnt = 0;
                    }
                    else {
                        info.state_cnt++;
                    }
                }
                else {
                    info.state_cnt = 0;
                }
            }
            break;
        default:
            std::cout << "[CLSF] Warning: Unknown State\n";
            break;
    }


/*
    if (max_ratio <= SLOT_THRESHOLD_RATIO) {
        if (curr_state != EMPTY)
            mgr.update_slot(slot_id, EMPTY);
        *prev_state = EMPTY;
    }
    else {
        if (curr_state == EMPTY) {
            mgr.update_slot(slot_id, OCCUPIED);
        }
        else {
//            if (info.bright >= HEADLIGHT_THRESHOLD) {
            if (max_bright >= HEADLIGHT_THRESHOLD) {
                if (*prev_state == EMPTY) {
                    if (curr_state != OCCUPIED)
                        mgr.update_slot(slot_id, OCCUPIED);
                }
                else {
                    if (curr_state != EXITING)
                        mgr.update_slot(slot_id, EXITING);
                    *prev_state = EXITING;
                }
            }
            else {
                if (*prev_state == EXITING) {
                }
                else {
                    if (curr_state != OCCUPIED)
                        mgr.update_slot(slot_id, OCCUPIED);
                    *prev_state = OCCUPIED;
                }
            }
        }
    }
*/

    // [Debug Session]
    if (slot_id == 1)
        std::cout << "[DEBUG][CLSF] updateState: Slot " << slot_id << ": curr_state=" << curr_state << ", state_cnt=" << info.state_cnt << ", bright=" << max_bright << "\n";
    // ---------------

}




void ParkingStatusClassifier::update(const int slot_id, float ratio, float bright) {

    auto it = slot_data.find(slot_id);
    if (it == slot_data.end()) {
        std::cerr << "[AI][CLSF] Warning: Invalid update slot id\n";
    }
    auto& info = it->second;

    if (info.ratios.size() >= NUM_SLOT_INFO)
        info.ratios.pop_front();
    info.ratios.push_back(ratio);

    if (info.brights.size() >= NUM_SLOT_INFO)
        info.brights.pop_front();
    info.brights.push_back(bright);
//    info.bright = bright;

    updateState(slot_id, info);

}


void ParkingStatusClassifier::classify(const cv::Mat& frame, const std::vector<Detection>& detections) {

    for (int i=0; i<map.total_slots; ++i) {
        const auto& slot = map.slots[i];
        float max_area = 0.0f;
        float ratio = 0.0f;
        std::vector<cv::Point> poly(slot.poly, slot.poly+4);
        float slot_area = std::fabs(cv::contourArea(poly));
        float bright = 0.0f;
        cv::Rect head_rect;

        for (const auto& detection : detections) {
            float area = compute_overlap_area(poly, detection.box);
            if (area > max_area) {
                max_area = area;
                bright = get_brightness(frame, detection.box, head_rect);
            }
        }
        ratio = max_area / slot_area;

//	std::cout << "[AI][CLSF] slot: " << slot.slot_id << " brg: " << bright << "\n";
        update(slot.slot_id, ratio, bright);

        // Debug Session
//        std::cout << "[AI][CLSF] Slot " << slot.slot_id
//            << ": " << max_area << ", ratio: " << ratio
//            << ", " << "state: " << slot.state << "\n";
        // ---
    }
}
