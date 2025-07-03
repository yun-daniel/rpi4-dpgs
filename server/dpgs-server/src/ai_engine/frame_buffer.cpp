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
    buffer->write_idx   = 0;
    buffer->read_idx    = 0;
    buffer->count       = 0;
    for (int i=0; i<BUFFER_SIZE; ++i) buffer->slots[i].valid = false;

    std::cout << "[FB] Success: FrameBuffer Initialize\n";

    return true;
}


bool FrameBuffer::push(const cv::Mat& frame) {
    if (!frame.isContinuous()) return false;
    size_t size = frame.total() * frame.elemSize();
    if (size > FRAME_MAX_SIZE) {
        std::cerr << "[FB] Warning: push: Invalid Frame Size\n";
        return false;
    }

    size_t w = buffer->write_idx.load();
    size_t r = buffer->read_idx.load();
    size_t c = buffer->count.load();
    size_t next_w = (w+1) % BUFFER_SIZE;

    // Buffer Full
    if (c == BUFFER_SIZE) {
        r = (r+1) % BUFFER_SIZE;
        buffer->read_idx.store(r);
        buffer->count.fetch_sub(1);
    }

    FrameSlot& slot = buffer->slots[w];
    slot.rows       = frame.rows;
    slot.cols       = frame.cols;
    slot.type       = frame.type();
    slot.data_size  = size;
    std::memcpy(slot.data, frame.data, size);
    slot.valid      = true;

    buffer->write_idx.store(next_w);
    buffer->count.fetch_add(1);
    return true;
}


bool FrameBuffer::pop(cv::Mat& frame) {
    size_t c = buffer->count.load();

    // Buffer Empty
    if (c == 0) {
        std::cerr << "[FB] Warning: pop: Empty Buffer\n";
        return false;
    }

    size_t r = buffer->read_idx.load();
    FrameSlot& slot = buffer->slots[r];
    if (!slot.valid) return false;

    cv::Mat temp(slot.rows, slot.cols, slot.type, slot.data);
    temp.copyTo(frame);
    slot.valid = false;

    buffer->read_idx.store((r+1) % BUFFER_SIZE);
    buffer->count.fetch_sub(1);
    return true;
}
