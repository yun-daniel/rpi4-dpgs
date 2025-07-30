#include "vp_engine.h"


VPEngine::VPEngine(CamStream& _cam_str, StrFrameBuffer& _fb)
    : cam_str(_cam_str), fb(_fb) {
}

VPEngine::~VPEngine() {
}


cv::Mat VPEngine::image_processing(cv::Mat resized) {
    
    cv::Size calib_size(1920, 1080);
    cv::Size resized_size = resized.size();

    double scale_x = static_cast<double>(resized_size.width) / calib_size.width;
    double scale_y = static_cast<double>(resized_size.height) / calib_size.height;

    // 왜곡 보정 + 이미지 잘리는 부분 최적 ROI로 
    cv::Mat cameraMatrix = (cv::Mat_<double>(3, 3) << 
        1186.7223133343819, 0, 986.21322182838969,
        0, 1184.8678265803778, 532.80754882406495,
        0, 0, 1);

    cv::Mat distCoeffs = (cv::Mat_<double>(5, 1) <<
        -0.41726111701793561,
         0.22383045048988037,
         0.00060775874503814112,
         0.00035311515556693833,
        -0.065315810341365299);
    

    // 1920x1080 -> 640x360으로 조정 
    cv::Mat scaled_cameraMatrix = cameraMatrix.clone();
    scaled_cameraMatrix.at<double>(0, 0) *= scale_x; // fx
    scaled_cameraMatrix.at<double>(0, 2) *= scale_x; // cx
    scaled_cameraMatrix.at<double>(1, 1) *= scale_y; // fy
    scaled_cameraMatrix.at<double>(1, 2) *= scale_y; // cy

    cv::Mat newCameraMatrix = cv::getOptimalNewCameraMatrix(
        scaled_cameraMatrix, distCoeffs, resized.size(), 0.0, resized.size());

    cv::Mat map1, map2;
    cv::initUndistortRectifyMap(
        scaled_cameraMatrix, distCoeffs, cv::Mat(),
        newCameraMatrix, resized.size(),
        CV_16SC2, map1, map2);

    cv::Mat output;
    cv::remap(resized, output, map1, map2, cv::INTER_LINEAR);
 
    return output;

    // 제한적 히스토그램 평활화(L(밝기와 관련된 부분만))
//    cv::Mat lab, clahe_result;
//    cv::cvtColor(output, lab, cv::COLOR_BGR2Lab);
//
//    std::vector<cv::Mat> lab_channels;
//    cv::split(lab,lab_channels);
//
//    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8,8));
//    clahe -> apply(lab_channels[0], lab_channels[0]);
//
//    cv::merge(lab_channels,lab);
//    cv::cvtColor(lab,clahe_result, cv::COLOR_Lab2BGR);
//
//    return clahe_result;
}


bool VPEngine::initialize() {
    std::cout << "[VPE] Start to initialize...\n";


    std::cout << "[VPE] Success: Video Processing Engine initialized\n";
    return true;
}


void VPEngine::run() {
    std::cout << "[VPE] Start Video Processing Engine\n";

    cv::Mat frame, resized, processed;

    is_running = true;
    while (is_running) {

        if (!cam_str.frame_sampling(frame)) {
            std::cerr << "[VPE] sampling: Failed to read frame from input stream.\n";
            break;
        }
        if (frame.empty()) {
            continue;
        }


        cv::resize(frame, resized, cv::Size(640, 360), 0, 0, cv::INTER_AREA);

        fb.push(resized);


//     #if ENABLE_DIST_CORRECTION
//        processed = image_processing(resized);
//
//        fb.push(processed);
//        clt_fb1->push(processed);
//     #else
//        fb.push(resized);
//        clt_fb1->push(resized);
//     #endif

        // [Debug Session]
        // Check input frame before pushing buffers
//        std::cout << "[VPE][DEBUG] Frame Info: rows: " << resized.rows << " cols: " << resized.cols << " size: " << resized.size << "\n";

//        cv::imshow("[VPE] resized", resized);
//        cv::waitKey(1);
//        cv::imwrite("for_mapgen.jpg", resized);
//        cv::imwrite("for_mapgen.jpg", processed);
        // ----------

    }

}


void VPEngine::stop() {
    std::cout << "[VPE] VPEngine Terminating...\n";

    is_running = false;

    clear();

    std::cout << "[VPE] VPEngine Terminated\n";
}

void VPEngine::clear() {
    std::cout << "[VPE] clear: Cleanning...\n";

//    delete csc;
//    csc = nullptr;
//
//    delete clt_fb1;
//    clt_fb1 = nullptr;
//
//    delete clt_fb2;
//    clt_fb2 = nullptr;

    std::cout << "[VPE] clear: Cleanning Success\n";
}


bool VPEngine::is_run() {
    return is_running;
}

//FrameBufferStr* VPEngine::get_clt_fb(int idx) {
//    if (idx == 1)
//        return clt_fb1;
//    else if (idx == 2)
//        return clt_fb2;
//    else
//        return nullptr;
//}
