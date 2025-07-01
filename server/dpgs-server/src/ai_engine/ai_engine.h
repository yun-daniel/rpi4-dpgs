#ifndef __AI_ENGINE_H__
#define __AI_ENGINE_H__

#include <opencv2/opencv.hpp>


void int_dnn(std::vector<std::string> &class_list, cv::dnn::Net &net);
void run_ai_engine();


#endif // __AI_ENGINE_H__
