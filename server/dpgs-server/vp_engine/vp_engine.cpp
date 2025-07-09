#include "vp_engine.hpp"
//#include "client_if.hpp"

queue<Mat> VPEngine::frame_queue;
mutex VPEngine::queue_mutex;
condition_variable VPEngine::queue_cv;

Mat VPEngine::image_processing(Mat resized){
    
    Size calib_size(1920, 1080);
    Size resized_size = resized.size();

    double scale_x = static_cast<double>(resized_size.width) / calib_size.width;
    double scale_y = static_cast<double>(resized_size.height) / calib_size.height;

    // 왜곡 보정 + 이미지 잘리는 부분 최적 ROI로 
    Mat cameraMatrix = (Mat_<double>(3, 3) << 
        1186.7223133343819, 0, 986.21322182838969,
        0, 1184.8678265803778, 532.80754882406495,
        0, 0, 1);

    Mat distCoeffs = (Mat_<double>(5, 1) <<
        -0.41726111701793561,
         0.22383045048988037,
         0.00060775874503814112,
         0.00035311515556693833,
        -0.065315810341365299);
    

    // 1920x1080 -> 640x360으로 조정 
    Mat scaled_cameraMatrix = cameraMatrix.clone();
    scaled_cameraMatrix.at<double>(0, 0) *= scale_x; // fx
    scaled_cameraMatrix.at<double>(0, 2) *= scale_x; // cx
    scaled_cameraMatrix.at<double>(1, 1) *= scale_y; // fy
    scaled_cameraMatrix.at<double>(1, 2) *= scale_y; // cy

    Mat newCameraMatrix = getOptimalNewCameraMatrix(
        scaled_cameraMatrix, distCoeffs, resized.size(), 0.0, resized.size());

    Mat map1, map2;
    initUndistortRectifyMap(
        scaled_cameraMatrix, distCoeffs, Mat(),
        newCameraMatrix, resized.size(),
        CV_16SC2, map1, map2);

    Mat output;
    remap(resized, output, map1, map2, INTER_LINEAR);
    
    // 제한적 히스토그램 평활화(L(밝기와 관련된 부분만))
    Mat lab, clahe_result;
    cvtColor(output, lab, COLOR_BGR2Lab);

    vector<Mat> lab_channels;
    split(lab,lab_channels);

    Ptr<CLAHE> clahe = createCLAHE(2.0, Size(8,8));
    clahe -> apply(lab_channels[0], lab_channels[0]);

    merge(lab_channels,lab);
    cvtColor(lab,clahe_result,COLOR_Lab2BGR);

    return clahe_result;
}

void VPEngine::send_image(const Mat& processed){
    lock_guard<mutex> lock(queue_mutex);
    frame_queue.push(processed.clone());
    queue_cv.notify_one();
}

void VPEngine::run_video_loop() {
    //string rtsp_input_pipeline = "rtspsrc location=rtsp://192.168.0.113:8554/test latency=30 ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! appsink";
    string rtsp_input_pipeline = "rtspsrc location=rtsp://admin:Veda123%21@192.168.0.86:554/profile2/media.smp latency=30 ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! appsink drop=true max-buffers=1";

    VideoCapture cap(rtsp_input_pipeline, CAP_GSTREAMER);
    if (!cap.isOpened()) {
        cerr << "Failed to open RTSP input stream." << endl;
        return;
    }

    Mat frame, resized, processed;
    int frame_count = 0;
    auto start = steady_clock::now();

    while (true) {
        if (!cap.read(frame)) {
            cerr << "Failed to read frame from input RTSP." << endl;
            break;
        }

        resize(frame, resized, Size(640, 360), 0, 0, INTER_AREA);
        imshow("RTSP Raw Video (CCTV -> RPI)", resized);
        
        processed = image_processing(resized);
        // resize(processed, processed, Size(640, 360), 0, 0, INTER_AREA);
        imshow("RTSP Processed Video (CCTV -> RPI)", processed);
        
        if (waitKey(1) == 27) break;

        //push_frame_to_rtsp(processed);

        frame_count++;
        auto now = steady_clock::now();
        auto elapsed = duration_cast<seconds>(now - start).count();
        if (elapsed >= 5) {
            double fps = frame_count / static_cast<double>(elapsed);
            cout << "Average FPS over last " << elapsed << " seconds: " << fps << endl;
            frame_count = 0;
            start = now;
        }

        send_image(processed);

    }
}

void VPEngine::run(){
    gst_init(nullptr, nullptr);
    run_video_loop();
}
