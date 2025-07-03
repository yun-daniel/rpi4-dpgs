#ifndef __FRAME_BUFFER_H__
#define __FRAME_BUFFER_H__

#include <atomic>
#include <opencv2/opencv.hpp>
#include <string>


constexpr int BUFFER_SIZE       = 5;
constexpr int FRAME_WIDTH       = 640;
constexpr int FRAME_HEIGHT      = 480;
constexpr int FRAME_CHANNELS    = 3;
constexpr int FRAME_MAX_SIZE    = FRAME_WIDTH * FRAME_HEIGHT * FRAME_CHANNELS;

struct FrameSlot {
    std::atomic<bool>   valid;
    int                 rows, cols, type;
    size_t              data_size;
    uint8_t             data[FRAME_MAX_SIZE];
};

struct SharedFrameBuffer {
    std::atomic<size_t> write_idx;
    std::atomic<size_t> read_idx;
    std::atomic<size_t> count;
    FrameSlot slots[BUFFER_SIZE];
};



class FrameBuffer {
 public:
    FrameBuffer(const std::string& _shm_name);
    ~FrameBuffer();

    bool initialize();
    bool push(const cv::Mat& frame);
    bool pop(cv::Mat& frame);


 private:
    std::string shm_name;
    int shm_fd              = -1;
    void* shm_ptr           = nullptr;
    size_t shm_total_size   = 0;

    SharedFrameBuffer* buffer = nullptr;

};



#endif // __FRAME_BUFFER_H__
