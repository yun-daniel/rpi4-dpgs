#include "device_manager.h"
#include "sender/sender.h"

#include <iostream>

DeviceManager::DeviceManager() {
    // 필요 시 초기화
}

void DeviceManager::run() {
    std::cout << "[DM] Running Device Manager\n";

    // map.json → 직접 파싱하는 sender
    Sender sender("map.json", "192.168.0.54");
    sender.run();
}
