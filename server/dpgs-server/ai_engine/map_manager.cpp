#include "map_manager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_set>


static std::string extract_string_value(const std::string& line) {
    auto begin = line.find('\"', line.find(':')) + 1;
    auto end = line.find('\"', begin);
    return line.substr(begin, end - begin);
}

static int extract_int_value(const std::string& line) {
    auto pos = line.find(':');
    return std::stoi(line.substr(pos + 1));
}

static std::vector<int> extract_box_values(const std::string& line) {
    std::vector<int> result;
    auto start = line.find('[');
    auto end = line.find(']');
    std::string content = line.substr(start + 1, end - start - 1);
    std::stringstream ss(content);
    std::string token;
    while (std::getline(ss, token, ',')) {
        result.push_back(std::stoi(token));
    }
    return result;
}


static std::vector<cv::Point> extract_poly_points(const std::string& line) {
    std::vector<cv::Point> points;

    auto start = line.find('[');
    auto end = line.rfind(']');
    std::string content = line.substr(start + 1, end - start - 1);

    std::stringstream ss(content);
    std::string point_str;
    while (std::getline(ss, point_str, ']')) {
        if (point_str.find('[') == std::string::npos) continue;

        auto x_start = point_str.find('[') + 1;
        auto comma = point_str.find(',', x_start);
        if (comma == std::string::npos) continue;

        int x = std::stoi(point_str.substr(x_start, comma - x_start));
        int y = std::stoi(point_str.substr(comma + 1));
        points.emplace_back(x, y);

        // consume next comma
        if (ss.peek() == ',') ss.ignore();
    }

    return points;
}



MapManager::MapManager() {
    std::cout << "[MAP] Create New MapDB\n";
    if (!newDB()) {
        std::cerr << "[MAP] Error: fail to create MapDB\n";
    }
}


MapManager::MapManager(const std::string& _path)
    : file_path(_path) {
    if (!load()) {
        std::cerr << "Error: fail to load Map DB: " << _path << "\n";
    }
    std::cout << "Loaded Map DB\n";
    printMap();
}


bool MapManager::newDB() {
    file_path = "config/tmp_map.json";
    map = ParkingLotMap();
    map.parking_lot_id = "Default";
    map.total_slots = map.slots.size();

//    save();

    return true;
}


bool MapManager::load() {
    std::ifstream file(file_path);
    if (!file.is_open()) return false;

    std::string line;
    Slot current_slot;
    bool in_slots = false;

    map = ParkingLotMap();

    while (std::getline(file, line)) {
        line.erase(0, line.find_first_not_of(" \t"));

        if (line.find("\"parking_lot_id\"") != std::string::npos) {
            map.parking_lot_id = extract_string_value(line);
        } else if (line.find("\"total_slots\"") != std::string::npos) {
            map.total_slots = extract_int_value(line);
        } else if (line.find("\"slots\"") != std::string::npos) {
            in_slots = true;
        } else if (line.find("{") != std::string::npos && in_slots) {
            current_slot = Slot();  // reset
        } else if (line.find("\"slot_id\"") != std::string::npos) {
            current_slot.slot_id = extract_int_value(line);
        } else if (line.find("\"state\"") != std::string::npos) {
            current_slot.state = extract_string_value(line);
        } else if (line.find("\"poly\"") != std::string::npos) {
            current_slot.poly = extract_poly_points(line);
            if (current_slot.poly.size() != 4) {
                std::cerr << "[MAP] Error: load: slot_id " << current_slot.slot_id << " has invalid poly (must have 4 points)\n";
                return false;
            }
        } else if (line.find("}") != std::string::npos && in_slots) {
            map.slots.push_back(current_slot);
        } else if (line.find("]") != std::string::npos && in_slots) {
            in_slots = false;
        }
    }

    return checksum();
}


bool MapManager::checksum() {
    bool valid = true;

    if ((int)map.slots.size() != map.total_slots) {
        std::cerr << "Error: total_slots (" << map.total_slots
            << ") doesn't match number of slots (" << map.slots.size() << ")\n";
        valid = false;
    }

    std::unordered_set<int> seen_ids;
    for (const auto& slot : map.slots) {
        if (seen_ids.count(slot.slot_id)) {
            std::cerr << "Error: Duplicate slot_id found: " << slot.slot_id << "\n";
            valid = false;
        }
        seen_ids.insert(slot.slot_id);

//        if (slot.box.x < 0 || slot.box.y < 0 || slot.box.width <= 0 || slot.box.height <= 0) {
//            std::cerr << "Error: Invalid box in slot_id " << slot.slot_id
//                << ": [" << slot.box.x << "," << slot.box.y
//                << "," << slot.box.width << "," << slot.box.height << "]\n";
//            valid = false;
//        }
    }

    return valid;
}


bool MapManager::insert_slot(int slot_id, const std::string& state, const std::vector<cv::Point>& poly) {
    for (const auto& slot : map.slots) {
        if (slot.slot_id == slot_id) {
            std::cerr << "Warning: insert_slot failed: slot_id " << slot_id << " already exists.\n";
            return false;
        }
    }

    Slot new_slot{slot_id, state, poly};
    map.slots.push_back(new_slot);
    map.total_slots = map.slots.size();

    return true;
}


bool MapManager::update_slot(int slot_id, const std::string& state) {
    for (auto& slot : map.slots) {
        if (slot.slot_id == slot_id) {
            slot.state = state;
            return true;
        }
    }

    std::cerr << "Warning: update_slot failed: slot_id " << slot_id << " not found.\n";

    return false;
}


bool MapManager::delete_slot(int slot_id) {
    auto it = std::remove_if(map.slots.begin(), map.slots.end(),
                            [slot_id](const Slot& s) { return s.slot_id == slot_id; });
    if (it == map.slots.end()) {
        std::cerr << "Warning: slot_id not found: " << slot_id << "\n";
        return false;
    }

    map.slots.erase(it, map.slots.end());
    map.total_slots = map.slots.size();

    return true;
}


void MapManager::sort_slots() {
    std::sort(map.slots.begin(), map.slots.end(),
        [](const Slot& a, const Slot& b) { return a.slot_id < b.slot_id; });
}


bool MapManager::save() {
    if (!checksum()) {
        std::cerr << "Error: Failed to save by checksum\n";
        return false;
    }
    sort_slots();

    std::ofstream file(file_path);
    if (!file.is_open()) return false;

    file << "{\n";
    file << "  \"parking_lot_id\": \"" << map.parking_lot_id << "\",\n";
    file << "  \"total_slots\": " << map.total_slots << ",\n";
    file << "  \"slots\": [\n";

    for (size_t i = 0; i < map.slots.size(); ++i) {
        const auto& s = map.slots[i];
        file << "    {\n";
        file << "      \"slot_id\": " << s.slot_id << ",\n";
        file << "      \"state\": \"" << s.state << "\",\n";
        file << "      \"poly\": [";
        for (size_t j=0; j<s.poly.size(); ++j) {
            const auto& pt = s.poly[j];
            file << "[" << pt.x << ", " << pt.y << "]";
            if (j != s.poly.size()-1) file << ", ";
        }
        file << "]\n";
        file << "    }";
        if (i != map.slots.size() - 1) file << ",";
        file << "\n";
    }

    file << "  ]\n";
    file << "}\n";

    return true;
}

void MapManager::printMap() {
    std::cout << "[Parking Lot ID]: " << map.parking_lot_id
        << " (Total Slots: " << map.total_slots << ")\n";
    for (const auto& slot : map.slots) {
        std::cout << "Slot " << slot.slot_id << ": " << slot.state << " at ";
        for (const auto& pt : slot.poly) {
            std::cout << "[" << pt.x << "," << pt.y << "] ";
        }
        std::cout << "\n";
    }
}

const ParkingLotMap& MapManager::getMap() const {
    return map;
}

