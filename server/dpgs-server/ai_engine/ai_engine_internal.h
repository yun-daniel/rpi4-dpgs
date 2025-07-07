#ifndef __AI_ENGINE_INTERNAL_H__
#define __AI_ENGINE_INTERNAL_H__

#include "ai_engine.h"

#include <opencv2/opencv.hpp>
#include <atomic>

#include "frame_buffer.h"
#include "map_manager.h"
#include "detector.h"
#include "classifier.h"


// YOLO Model Settings
const std::string   ONNX_FILE               = "config/yolov5s_640.onnx";
const std::string   CLASS_LIST_FILE         = "config/classes.txt";
constexpr int       INPUT_WIDTH             = 640;
constexpr int       INPUT_HEIGHT            = 640;
constexpr int       OUTPUT_DIM              = 85;
constexpr int       ANCHORS_PER_CELL        = 3;
constexpr int       STRIDE_1                = 8;
constexpr int       STRIDE_2                = 16;
constexpr int       STRIDE_3                = 32;
constexpr int       TOTAL_PREDICTIONS =
    ANCHORS_PER_CELL * (
        (INPUT_WIDTH * INPUT_HEIGHT) / (STRIDE_1 * STRIDE_1) +
        (INPUT_WIDTH * INPUT_HEIGHT) / (STRIDE_2 * STRIDE_2) +
        (INPUT_WIDTH * INPUT_HEIGHT) / (STRIDE_3 * STRIDE_3)
    );
const float         SCORE_THRESHOLD         = 0.2;
const float         NMS_THRESHOLD           = 0.4;
const float         CONFIDENCE_THRESHOLD    = 0.4;


const std::vector<cv::Scalar> colors = {
                                    cv::Scalar(0, 255, 0),      // EMPTY
                                    cv::Scalar(0, 0, 255),      // OCCUPIED
                                    cv::Scalar(0, 165, 255),    // EXITING
                                    cv::Scalar(255, 255, 0),
                                    cv::Scalar(0, 255, 255),
                                    cv::Scalar(255, 0, 0)};


class AIEngine::Impl {
 public:
    Impl(FrameBuffer& _fb, MapManager& _mgr);
    ~Impl();

    void run();
    void stop();

 private:
    std::vector<std::string>    load_class_list();
    void                        init_dnn();
    cv::Mat                     frame_sampling();
    
    std::atomic<bool>           is_running;
    std::vector<std::string>    class_list;
    cv::dnn::Net                net;

    // External Interface
    FrameBuffer&                fb;
    MapManager&                 mgr;

    // For Debug
    void overlay_slots(cv::Mat& frame, const SharedParkingLotMap& map);
    void print_frame(const cv::Mat& frame);

    // For Test
    bool init_test_video();
    cv::VideoCapture test_video;
};



#endif // __AI_ENGINE_INTERNAL_H__
