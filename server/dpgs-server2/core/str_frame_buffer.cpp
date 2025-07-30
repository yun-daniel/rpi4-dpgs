#include "str_frame_buffer.h"


StrFrameBuffer::StrFrameBuffer() {
    max_size = STR_BUFFER_SIZE;
}

StrFrameBuffer::~StrFrameBuffer() {

    sem_destroy(&sem_ready);

}


bool StrFrameBuffer::initialize() {

    sem_init(&sem_ready, 0, 0);

    return true;
}


void StrFrameBuffer::push(const cv::Mat& frame) {
    std::unique_lock<std::mutex> lock(sem_mutex);
	
    if (frame.empty()) {
        std::cout << "[FB_STR] Empty Frame pushed\n";
        return;
    }
	cv::Mat cloned = frame.clone();

    if (buffer.size() >= max_size) {
        buffer.pop_front();
    }

    buffer.push_back(cloned);

    // [Debug Session]
    cv::imshow("[FB] push", cloned);
    cv::waitKey(1);

    sem_post(&sem_ready);
}


cv::Mat StrFrameBuffer::pop() {
    sem_wait(&sem_ready);

    std::unique_lock<std::mutex> lock(sem_mutex);
    if (notified) {
        notified = false;
        return cv::Mat();
    }
    if (buffer.empty()) {
        std::cout << "[FB_STR] Buffer is empty\n";
        return cv::Mat();
    }

    cv::Mat frame = buffer.front();
    cv::Mat cloned = frame.clone();

    // [Debug Session]
//    cv::imshow("[FB] pop", cloned);
//    cv::waitKey(1);


	return cloned;
}


void StrFrameBuffer::notify() {
    notified = true;
    sem_post(&sem_ready);
}
