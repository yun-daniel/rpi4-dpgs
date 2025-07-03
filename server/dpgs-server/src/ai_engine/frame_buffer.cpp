#include "frame_buffer.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <iostream>


FrameBuffer::FrameBuffer(const std::string& _shm_name)
    : shm_name(_shm_name) {
}

FrameBuffer::~FrameBuffer() {
    if (shm_ptr) munmap(shm_ptr, shm_total_size);
    if (shm_fd >= 0) close (shm_fd);
}


bool FrameBuffer::initialize() {
    shm_total_size = sizeof(SharedFrameBuffer);

    shm_fd = shm_open(shm_name.c_str(), O_CREAT | O_RDWR, 0666);
    if (shm_fd < 0) {
        std::cerr << "[FB] Error: Fail to shm_open\n";
        return false;
    }

    if (ftruncate(shm_fd, shm_total_size) != 0) {
        std::cerr << "[FB] Error: Fail to ftruncate\n";
        return false;
    }

    shm_ptr = mmap(nullptr, shm_total_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        std::cerr << "[FB] Error: Fail to mmap\n";
        return false;
    }

    buffer = reinterpret_cast<SharedFrameBuffer*>(shm_ptr);
    buffer->write_idx = 0;
    buffer->read_idx = 0;
    for (int i=0; i<BUFFER_SIZE; ++i) buffer->slots[i].ready = false;

    std::cout << "[FB] Success: FrameBuffer Initialize\n";

    return true;
}


bool FrameBuffer::push(const cv::Mat& frame) {
    if (!frame.isContinuous()) return false;
    size_t size = frame.total() * frame.elemSize();
    if (size > FRAME_MAX_SIZE) return false;

    size_t w = buffer->write_idx.load();
    size_t next_w = (w+1) % BUFFER_SIZE;
    if (next_w == buffer->read_idx.load()) return false;    // Full buffer

    FrameSlot& slot = buffer->slots[w];
    slot.rows       = frame.rows;
    slot.cols       = frame.cols;
    slot.type       = frame.type();
    slot.data_size  = size;
    std::memcpy(slot.data, frame.data, size);
    slot.ready = true;

    buffer->write_idx.store(next_w);
    return true;
}


bool FrameBuffer::pop(cv::Mat& frame) {
    size_t r = buffer->read_idx.load();
    if (r == buffer->write_idx.load()) return false;        // Empty buffer

    FrameSlot& slot = buffer->slots[r];
    if (!slot.ready) return false;

    cv::Mat temp(slot.rows, slot.cols, slot.type, slot.data);
    temp.copyTo(frame);
    slot.ready = false;

    buffer->read_idx.store((r+1) % BUFFER_SIZE);
    return true;
}
