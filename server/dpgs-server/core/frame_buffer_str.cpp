#include "frame_buffer_str.h"


FrameBufferStr::FrameBufferStr() {
    max_size = STR_BUFFER_SIZE;
}

FrameBufferStr::~FrameBufferStr() {

    sem_destroy(&sem_ready);

}


bool FrameBufferStr::initialize() {

    sem_init(&sem_ready, 0, 0);

    return true;
}


void FrameBufferStr::push(const cv::Mat& frame) {
//    std::cout << "[FB_STR][DEBUG] push called\n";
    std::unique_lock<std::mutex> lock(sem_mutex);
	
    if (frame.empty()) {
        std::cout << "[FB_STR] Empty Frame pushed\n";
        return;
    }
	cv::Mat cloned = frame.clone();

    if (buffer.size() >= max_size) {
        buffer.pop_front();
    }
//	std::cout << "[FB_STR][DEBUG] push called2\n";

//    cv::imshow("FB_STR_before_cloned", cloned);
    buffer.push_back(cloned);
//    cv::imshow("FB_STR_cloned", cloned);

    sem_post(&sem_ready);
}


cv::Mat FrameBufferStr::pop() {
//    std::cout << "[FB_STR][DEBUG] pop called\n";
    sem_wait(&sem_ready);

    std::unique_lock<std::mutex> lock(sem_mutex);
    if (buffer.empty()) {
        std::cout << "[FB_STR] Buffer is empty\n";
        return cv::Mat();
    }


    if (notified) {
        notified = false;
        return cv::Mat();
    }

//    	std::cout << "[FB_STR][DEBUG] pop test1\n";
    cv::Mat frame = buffer.front();
//    	std::cout << "[FB_STR][DEBUG] pop test2\n";
    cv::Mat cloned = frame.clone();
//    	std::cout << "[FB_STR][DEBUG] pop test3\n";
//    cv::imshow("FB_STR_POP", frame);
//    cv::imshow("FB_STR_POP_cloned", cloned);
//    	std::cout << "[FB_STR][DEBUG] pop test4\n";

	return cloned;
//    return frame;
}


void FrameBufferStr::notify() {
    notified = true;
    sem_post(&sem_ready);
}
