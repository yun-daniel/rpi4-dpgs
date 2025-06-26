#ifndef __AI_ENGINE_H__
#define __AI_ENGINE_H__

#include <opencv2/opencv.hpp>


struct Detection {
    int class_id;
    float confidence;
    cv::Rect box;
};



std::vector<std::string> load_class_list();
void int_dnn(std::vector<std::string> &class_list, cv::dnn::Net &net);
cv::Mat letterbox(const cv::Mat &src);
void detector(cv::Mat &_frame, cv::dnn::Net &net, std::vector<Detection> &output, const std::vector<std::string> &class_name);

void run_ai_engine();


#endif // __AI_ENGINE_H__
