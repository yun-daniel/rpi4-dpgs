#include "vp_engine.h"


VPEngine::VPEngine(FrameBuffer& _fb)
    : fb(_fb) {

}

VPEngine::~VPEngine() {
}


bool VPEngine::initialize() {
    std::cout << "[VPE] Start to initialize...\n";

    csc = new CamStreamingClient();
    if (!csc->initialize()) {
        std::cerr << "[VPE] Error: Failed to initialize Camera Streaming Client\n";
        return false;
    }

    clt_fb1 = new FrameBufferStr();
    if (!clt_fb1->initialize()) {
        std::cerr << "[VPE] Error: Failed to initialize Streaming Frame Buffer1\n";
        return false;
    }
    clt_fb2 = new FrameBufferStr();
    if (!clt_fb2->initialize()) {
        std::cerr << "[VPE] Error: Failed to initialize Streaming Frame Buffer2\n";
        return false;
    }

    std::cout << "[VPE] Success: Video Processing Engine initialized\n";
    return true;
}


void VPEngine::run() {
    std::cout << "[VPE] Start Video Processing Engine\n";

    cv::Mat frame, resized, processed;

    is_running = true;
    while (is_running) {

        if (!csc->frame_sampling(frame)) {
            std::cerr << "[VPE] sampling: Failed to read frame from input stream.\n";
            break;
        }
        if (frame.empty()) {
            continue;
        }


        cv::resize(frame, resized, cv::Size(640, 360), 0, 0, cv::INTER_AREA);

        std::cout << "[VPE][DEBUG] Frame Info: rows: " << resized.rows << " cols: " << resized.cols << " size: " << resized.size << "\n";

        fb.push(resized);
        clt_fb1->push(resized);

/*
	{
	std::lock_guard<std::mutex> lock(queue_mutex_1);
	if (frame_queue_1.size() > 30) {
		frame_queue_1.pop();
	}
	frame_queue_1.push(resized.clone());
	queue_cv_1.notify_one();
	}
*/



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

    delete csc;
    csc = nullptr;

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
