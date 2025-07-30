#ifndef __STREAMING_FRAME_BUFFER_H__
#define __STREAMING_FRAME_BUFFER_H__

#include <opencv2/opencv.hpp>
#include <deque>
#include <mutex>
#include <semaphore.h>
#include <atomic>

constexpr int STR_BUFFER_SIZE       = 5;


class StrFrameBuffer {
 public:
    StrFrameBuffer();
    ~StrFrameBuffer();

    bool initialize();

    void push(const cv::Mat& frame);
    cv::Mat pop();
    void notify();

 private:
    std::deque<cv::Mat> buffer;
    size_t max_size;

    std::mutex  sem_mutex;
    sem_t       sem_ready;
    std::atomic<bool> notified;

};



#endif // __STREAMING_FRAME_BUFFER_H__
