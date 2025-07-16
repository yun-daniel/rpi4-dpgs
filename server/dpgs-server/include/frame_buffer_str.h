#ifndef __FRAME_BUFFER_STR_H__
#define __FRAME_BUFFER_STR_H__

#include <opencv2/opencv.hpp>
#include <deque>
#include <mutex>
#include <semaphore.h>
#include <atomic>

constexpr int STR_BUFFER_SIZE       = 5;


class FrameBufferStr {
 public:
    FrameBufferStr();
    ~FrameBufferStr();

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



#endif // __FRAME_BUFFER_STR_H__
