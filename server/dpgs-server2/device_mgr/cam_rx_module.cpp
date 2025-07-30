#include "cam_rx_module.h"


CamRxModule::CamRxModule(const std::string& _stream_src, StrFrameBuffer& _fb)
    : stream_src(_stream_src), fb(_fb) {
}

CamRxModule::~CamRxModule() {
}


bool CamRxModule::initialize() {
    std::cout << "[CAM_RX] Start to initialize...\n";

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "[CAM_RX] Error: Cannot create socket\n";
        return false;
    }

    memset(&cam_addr, 0, sizeof(cam_addr));
    cam_addr.sin_family = AF_INET;
    cam_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, cam_ip.c_str(), &cam_addr.sin_addr) <= 0) {
        std::cerr << "[CAM_RX] Invalid address/ Address not supported\n";
        return false;
    }


    gst_init(nullptr, nullptr);



    std::cout << "[CAM_RX] Success: Camera Receiver Module Initialized\n";
    return true;
}


void CamRxModule::run() {
    std::cout << "[CAM_RX] Run Camera Receive Module\n";

//    if (connect(sock, (struct sockaddr*)&cam_addr, sizeof(cam_addr)) < 0) {
//        std::cerr << "[CAM_RX] Connection Failed\n";
//        return;
//    }
//    std::cout << "[CAM_RX] Connected to cam_srv " << cam_ip << ":" << port << "\n";
//
//    cap.open(stream_src, cv::CAP_GSTREAMER);
    cap.open("tb/test_a.mp4");
    if (!cap.isOpened()) {
        std::cerr << "[CAM_RX] Error: Failed to open input stream: " << stream_src << "\n";
        return;
    }


    is_running = true;
    while (is_running) {

        cv::Mat frame;
        if (!cap.read(frame)) {
            std::cerr << "[CAM_RX] sampling: Failed to read frame from source\n";
            continue;
        }

        fb.push(frame);


        // [Debug Session]
//        cv::imshow("[CAM_RX] frame", frame);
//        cv::waitKey(1);
    }

    std::cout << "[CAM_RX] Camera Receive Module Terminated\n";

    close(sock);
}


void CamRxModule::stop() {
    std::cout << "[CAM_RX] Camera Receive Module Terminating...\n";

    is_running = false;

}

