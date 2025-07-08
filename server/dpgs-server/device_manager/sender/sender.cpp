#include "sender.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

Sender::Sender(const std::string& json_path, const std::string& target_ip, int port)
    : json_path(json_path)
{
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("[Sender] 소켓 생성 실패");
        return;
    }

    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(port);
    inet_pton(AF_INET, target_ip.c_str(), &target_addr.sin_addr);
}

Sender::~Sender() {
    if (sock_fd >= 0) close(sock_fd);
}

void Sender::run() {
    auto slots = parse_slots();
    if (slots.empty()) {
        std::cerr << "[Sender] 슬롯 정보 없음\n";
        return;
    }
    send_all(slots);
}

std::string Sender::read_file() {
    std::ifstream file(json_path);
    if (!file) {
        std::cerr << "[Sender] JSON 파일 열기 실패: " << json_path << "\n";
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::vector<Slot> Sender::parse_slots() {
    std::vector<Slot> slots;
    std::string content = read_file();
    std::regex slot_pattern(R"(\{\s*"slot_id"\s*:\s*(\d+),\s*"state"\s*:\s*(\d+))");
    std::smatch match;

    auto begin = content.cbegin();
    auto end = content.cend();
    while (std::regex_search(begin, end, match, slot_pattern)) {
        slots.push_back({std::stoi(match[1]) - 1, std::stoi(match[2])});
        begin = match.suffix().first;
    }

    return slots;
}

void Sender::send_packet(const Slot& slot) {
    std::string cmd = std::to_string(slot.slot_id) + " " + std::to_string(slot.state);
    sendto(sock_fd, cmd.c_str(), cmd.size(), 0, (struct sockaddr*)&target_addr, sizeof(target_addr));
    std::cout << "[SEND] 슬롯 " << slot.slot_id << " 상태 → " << slot.state << "\n";
}

void Sender::send_all(const std::vector<Slot>& slots) {
    for (const auto& slot : slots) {
        send_packet(slot);
    }
}
