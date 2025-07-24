#ifndef __AI_ENGINE_CLASSIFIER_H__
#define __AI_ENGINE_CLASSIFIER_H__

#include "detector.h"
#include "map_manager.h"
#include <opencv2/opencv.hpp>
#include <deque>
#include <vector>
#include <algorithm>


struct SlotInfo {
    std::deque<float>   ratios;
//    float               bright;
    std::deque<float>   brights;
    SlotState           prev_state = OCCUPIED;
    unsigned int        state_cnt = 0;
};


class ParkingStatusClassifier {
 public:
    ParkingStatusClassifier(MapManager& _mgr);

    void classify(const cv::Mat& frame, const std::vector<Detection>& detections);

 private:
    std::unordered_map<int, SlotInfo> slot_data;
    MapManager& mgr;
    const SharedParkingLotMap& map;

    void updateState(int slot_id, SlotInfo& info);
    void update(const int slot_id, float ratio, float brightness);


};


#endif // __AI_ENGINE_CLASSIFIER_H__
