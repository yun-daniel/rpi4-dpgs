#ifndef __FRAME_BUFFER_H__
#define __FRAME_BUFFER_H__

#include <atomic>
#include <opencv2/opencv.hpp>
#include <string>
#include <semaphore.h>


constexpr int BUFFER_SIZE       = 5;
constexpr int FRAME_WIDTH       = 1792; //1920; // 640;
constexpr int FRAME_HEIGHT      = 1344; //1080; //480;
constexpr int FRAME_CHANNELS    = 3;
constexpr int FRAME_MAX_SIZE    = FRAME_WIDTH * FRAME_HEIGHT * FRAME_CHANNELS;

struct FrameSlot {
    bool    valid;
    int     rows, cols, type;
    size_t  data_size;
    uint8_t data[FRAME_MAX_SIZE];
};

struct SharedFrameBuffer {
    size_t      write_idx;
    size_t      read_idx;
    size_t      count;
    FrameSlot   slots[BUFFER_SIZE];

    sem_t sem_ready;
    sem_t sem_mutex;
};



class FrameBuffer {
 public:
    FrameBuffer(const std::string& _shm_name);
    ~FrameBuffer();

    bool initialize();
    bool push(const cv::Mat& frame);
    bool pop(cv::Mat& frame);
    void destroyShm();


 private:
    std::string shm_name;
    int shm_fd              = -1;
    void* shm_ptr           = nullptr;
    size_t shm_total_size   = 0;

    SharedFrameBuffer* buffer = nullptr;

};



#endif // __FRAME_BUFFER_H__
