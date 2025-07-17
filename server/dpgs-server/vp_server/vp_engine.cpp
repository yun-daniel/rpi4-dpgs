#include "vp_engine.hpp"
//#include "client_if.hpp"

queue<Mat> VPEngine::frame_queue_1;
queue<Mat> VPEngine::frame_queue_2;

mutex VPEngine::queue_mutex_1;
mutex VPEngine::queue_mutex_2;

condition_variable VPEngine::queue_cv_1;
condition_variable VPEngine::queue_cv_2;

VPEngine::VPEngine(){
    //gstreamer 초기화
    gst_init(nullptr,nullptr);
    cctv_input_streaming_pipeline = "rtspsrc location=rtsp://admin:Veda123%21@192.168.0.86:554/profile2/media.smp"
                                    " latency=30 ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! appsink drop=true max-buffers=1";
    
}

VPEngine::~VPEngine(){}

Mat VPEngine::image_processing(Mat input_image){
    // [카메라 왜곡보정] 1920x1080 -> 640x360으로 조정
    Mat resized_image;
    resize(input_image, resized_image, Size(640, 360), 0, 0, INTER_AREA);
    
    Size calib_size(1920, 1080);
    Size resized_size = resized_image.size();
    
    double scale_x = static_cast<double>(resized_size.width) / calib_size.width;
    double scale_y = static_cast<double>(resized_size.height) / calib_size.height;

    Mat camera_matrix = (Mat_<double>(3, 3) << 
        1186.7223133343819, 0, 986.21322182838969,
        0, 1184.8678265803778, 532.80754882406495,
        0, 0, 1);
    
    Mat distortion_coeff = (Mat_<double>(5, 1) <<
        -0.41726111701793561,
         0.22383045048988037,
         0.00060775874503814112,
         0.00035311515556693833,
        -0.065315810341365299);

    Mat scaled_camera_matrix = camera_matrix.clone();
    scaled_camera_matrix.at<double>(0, 0) *= scale_x; // fx
    scaled_camera_matrix.at<double>(0, 2) *= scale_x; // cx
    scaled_camera_matrix.at<double>(1, 1) *= scale_y; // fy
    scaled_camera_matrix.at<double>(1, 2) *= scale_y; // cy
    
    Mat new_camera_matrix = getOptimalNewCameraMatrix(
        scaled_camera_matrix, distortion_coeff, resized_image.size(), 0.0, resized_image.size());

    Mat map1, map2;
    initUndistortRectifyMap(
        scaled_camera_matrix, distortion_coeff, Mat(),
        new_camera_matrix, resized_image.size(),
        CV_16SC2, map1, map2);

    Mat output;
    remap(resized_image, output, map1, map2, INTER_LINEAR);
    
    // [밝기 개선] 제한적 히스토그램 평활화(L(밝기와 관련된 부분만))
    Mat lab, clahe_result;
    cvtColor(output, lab, COLOR_BGR2Lab);

    vector<Mat> lab_channels;
    split(lab,lab_channels);

    Ptr<CLAHE> clahe = createCLAHE(2.0, Size(8,8));
    clahe -> apply(lab_channels[0], lab_channels[0]);

    merge(lab_channels,lab);
    cvtColor(lab,clahe_result,COLOR_Lab2BGR);

    imshow("[VP_ENGINE] Processed Video (CCTV -> RPI)", clahe_result);
    waitKey(1);

    return clahe_result;
}

void VPEngine::send_image(const Mat& processed){
    {
        //cout << "[Debug] processed: " << processed.empty() << endl;
        lock_guard<mutex> lock(queue_mutex_1);
        if(frame_queue_1.size() > MAX_QUEUE_SIZE){
            frame_queue_1.pop();
        }
        frame_queue_1.push(processed.clone());
        queue_cv_1.notify_one();
        // cout << "[VPEngine] [queue 1] push Video frame." << endl;
    
    } // block을 빠져나오면 queue_mutex_1에 대한 unlock

    {
        lock_guard<mutex> lock(queue_mutex_2);
        // frame_queue_2.push(Mat());
        // queue_cv_2.notify_one();
        // cout << "[VPEngine] [queue 2] push empty frame." << endl;
    
    } // block을 빠져나오면 queue_mutex_2에 대한 unlock

}

void VPEngine::start() {
    Mat frame, processed_image;
    int frame_count = 0;
    auto start = steady_clock::now();

    VideoCapture cap(cctv_input_streaming_pipeline, CAP_GSTREAMER);

    if (!cap.isOpened()) {
        cerr << "Failed to open RTSP input stream." << endl;
        return;
    }

    while (true) {
        if (!cap.read(frame)) {
            cerr << "Failed to read frame from input RTSP." << endl;
            break;
        }
        
        processed_image = image_processing(frame);
        
        //FPS 계산
        frame_count++;
        auto now = steady_clock::now();
        auto elapsed = duration_cast<seconds>(now - start).count();
        if (elapsed >= 5) {
            double fps = frame_count / static_cast<double>(elapsed);
            cout << "Average FPS over last " << elapsed << " seconds: " << fps << endl;
            frame_count = 0;
            start = now;
        }

        send_image(processed_image);

    }
}

void VPEngine::run(){
    cout << "[VPEngine Run]" << endl;
    start();
}
