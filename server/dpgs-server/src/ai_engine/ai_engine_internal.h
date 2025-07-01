#ifndef __AI_ENGINE_INTERNAL_H__
#define __AI_ENGINE_INTERNAL_H__

#include <opencv2/opencv.hpp>


const float INPUT_WIDTH = 640.0;
const float INPUT_HEIGHT = 640.0;
const float SCORE_THRESHOLD = 0.2;
const float NMS_THRESHOLD = 0.4;
const float CONFIDENCE_THRESHOLD = 0.4;

const std::vector<cv::Scalar> colors = {cv::Scalar(255, 255, 0), cv::Scalar(0, 255, 0), cv::Scalar(0, 255, 255), cv::Scalar(255, 0, 0), cv::Scalar(0, 0, 255)};


std::vector<std::string> load_class_list();
void print_frame(cv::Mat &_frame, const std::vector<std::string> &class_list);



#endif // __AI_ENGINE_INTERNAL_H__
