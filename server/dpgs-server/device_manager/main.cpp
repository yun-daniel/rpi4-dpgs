#include "device_manager.h"

#include <iostream>
#include <thread>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    DeviceManager dm;

    // 쓰레드로 run() 실행
    std::thread dm_thread(&DeviceManager::run, &dm);

    // 메인에서 다른 작업 가능 (필요 없으면 대기)
    std::cout << "[Main] Device Manager thread started\n";

    // dm_thread가 끝날 때까지 기다림
    dm_thread.join();

    std::cout << "[Main] Device Manager thread finished\n";
    return 0;
}
