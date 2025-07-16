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
    std::unique_lock<std::mutex> lock(sem_mutex);

    if (buffer.size() >= max_size) {
        buffer.pop_front();
    }

    buffer.push_back(frame.clone());

    sem_post(&sem_ready);
}


cv::Mat FrameBufferStr::pop() {
    sem_wait(&sem_ready);

    if (notified) {
        notified = false;
        return cv::Mat();
    }

    std::unique_lock<std::mutex> lock(sem_mutex);
    cv::Mat frame = buffer.front();
//    buffer.pop_front();

    return frame;
}


void FrameBufferStr::notify() {
    notified = true;
    sem_post(&sem_ready);
}
