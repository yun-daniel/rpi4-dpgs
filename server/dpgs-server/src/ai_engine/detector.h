#ifndef __AI_ENGINE_DETECTOR_H__
#define __AI_ENGINE_DETECTOR_H__

#include <opencv2/opencv.hpp>


struct Detection {
    int class_id;
    float confidence;
    cv::Rect box;
};


cv::Mat letterbox(const cv::Mat &src);
void detector(cv::Mat &_frame, cv::dnn::Net &net, std::vector<Detection> &output, const std::vector<std::string> &class_name);
std::vector<Detection> post_detector(cv::Mat &frame, std::vector<cv::Mat> &outputs, const std::vector<std::string> &className);


// Utility
void overlay_detection(cv::Mat& frame, std::vector<Detection> &outputs, const std::vector<std::string> &class_list);



#endif // __AI_ENGINE_DETECTOR_H__
