#ifndef __AI_ENGINE_CLASSIFIER_H__
#define __AI_ENGINE_CLASSIFIER_H__

#include "detector.h"
#include "map_manager.h"
#include <opencv2/opencv.hpp>


void classifier(const std::vector<Detection>& detections, MapManager& mgr);



#endif // __AI_ENGINE_CLASSIFIER_H__
