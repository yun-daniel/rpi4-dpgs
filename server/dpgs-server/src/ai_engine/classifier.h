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
};


class ParkingStatusClassifier {
 public:
    ParkingStatusClassifier(MapManager& _mgr);

    void classify(const std::vector<Detection>& detections);

 private:
    std::unordered_map<int, SlotInfo> slot_data;
    MapManager& mgr;
    const ParkingLotMap& map;


    void updateState(int slot_id, SlotInfo& info);
    void update(const int slot_id, float ratio);

};


#endif // __AI_ENGINE_CLASSIFIER_H__
