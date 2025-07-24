#include "map_manager.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_set>


// ------ Utility ------
static std::string extract_string_value(const std::string& line) {
    auto begin = line.find('\"', line.find(':')) + 1;
    auto end = line.find('\"', begin);
    return line.substr(begin, end - begin);
}

static int extract_int_value(const std::string& line) {
    auto pos = line.find(':');
    return std::stoi(line.substr(pos + 1));
}


static bool extract_poly_points(const std::string& line, cv::Point out[4]) {
    int count = 0;
    auto start = line.find('[');
    auto end = line.rfind(']');
    if (start == std::string::npos || end == std::string::npos || end <= start) return false;

    std::string content = line.substr(start + 1, end - start - 1);
    std::stringstream ss(content);
    std::string point_str;

    while (std::getline(ss, point_str, ']')) {
        if (point_str.find('[') == std::string::npos) continue;
        auto x_start = point_str.find('[') + 1;
        auto comma = point_str.find(',', x_start);
        if (comma == std::string::npos) return false;

        int x = std::stoi(point_str.substr(x_start, comma - x_start));
        int y = std::stoi(point_str.substr(comma + 1));
        if (count >= 4) return false;

        out[count++] = cv::Point(x, y);
        if (ss.peek() == ',') ss.ignore();
    }

    return (count == 4);
}
// -----------


// === MapManager ===
MapManager::MapManager(const std::string& _shm_name, const std::string& _file_path)
    : shm_name(_shm_name), file_path(_file_path) {
}

MapManager::~MapManager() {
    if (shm_ptr) {
        munmap(shm_ptr, shm_total_size);
        map = nullptr;
    }
    if (shm_fd >= 0) close(shm_fd);
}


bool MapManager::initialize() {
    std::cout << "[MM] Start to initialize...\n";

    shm_total_size = sizeof(SharedParkingLotMap);

    shm_fd = shm_open(shm_name.c_str(), O_CREAT | O_RDWR, 0666);
    if (shm_fd < 0) {
        std::cerr << "[MM] Error: Failed to shm_open\n";
        return false;
    }

    if (ftruncate(shm_fd, shm_total_size) != 0) {
        std::cerr << "[MM] Error: Failed to ftruncate\n";
        close(shm_fd);
        return false;
    }

    shm_ptr = mmap(nullptr, shm_total_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        std::cerr << "[MM] Error: Failed to mmap\n";
        close(shm_fd);
        return false;
    }


    map = reinterpret_cast<SharedParkingLotMap*>(shm_ptr);
    if (!load_map_data()) {
        std::cerr << "[MM] Error: Failed to load map data\n";
        close(shm_fd);
        return false;
    }


    pthread_mutexattr_t mtx_attr;
    pthread_mutexattr_init(&mtx_attr);
    pthread_mutexattr_setpshared(&mtx_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&map->mutex_map_dev, &mtx_attr);
    pthread_mutex_init(&map->mutex_map_clt, &mtx_attr);

    pthread_condattr_t cond_attr;
    pthread_condattr_init(&cond_attr);
    pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&map->cv_map_dev, &cond_attr);
    pthread_cond_init(&map->cv_map_clt, &cond_attr);

    map->flag_map_dev = false;
    map->flag_map_clt = false;


    sem_init(&map->sem_mutex, 1, 1);

    std::cout << "[MM] Success: MapManager Initialized\n";
    printMap();

    return true;
}


bool MapManager::load_map_data() {
    if (map == nullptr) {
        std::cerr << "[MM] Error: load_map_data: Doesn't be initialized\n";
        return false;
    }
    std::memset(map, 0, sizeof(SharedParkingLotMap));

    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "[MM] Error: Cannot open map file, " << file_path << "\n";
        return false;
    }

    std::string line;
    int slot_idx = 0;
    bool in_slots = false;

    while (std::getline(file, line)) {
        line.erase(0, line.find_first_not_of(" \t"));

        if (line.find("\"parking_lot_id\"") != std::string::npos) {
            auto val = extract_string_value(line);
            std::strncpy(map->parking_lot_id, val.c_str(), sizeof(map->parking_lot_id)-1);
        } else if (line.find("\"total_slots\"") != std::string::npos) {
            map->total_slots = extract_int_value(line);
        } else if (line.find("\"slots\"") != std::string::npos) {
            in_slots = true;
        } else if (line.find("{") != std::string::npos && in_slots) {
            if (slot_idx >= SLOTS_MAX_SIZE) {
                std::cerr << "[MM] Error: load: slot count exceeds SLOTS_MAX_SIZe\n";
                return false;
            }
        } else if (line.find("\"slot_id\"") != std::string::npos) {
            map->slots[slot_idx].slot_id = extract_int_value(line);         // TODO: int -> char
        } else if (line.find("\"state\"") != std::string::npos) {
            map->slots[slot_idx].state = static_cast<SlotState>(extract_int_value(line));
        } else if (line.find("\"poly\"") != std::string::npos) {
            if (!extract_poly_points(line, map->slots[slot_idx].poly)) {
                std::cerr << "[MM] Error: load: invalid poly for slot"
                        << map->slots[slot_idx].slot_id << "\n";
                return false;
            }
        } else if (line.find("}") != std::string::npos && in_slots) {
            ++slot_idx;
        } else if (line.find("]") != std::string::npos && in_slots) {
            in_slots = false;
        }
    }

    map->total_slots = slot_idx;

    return checksum();
}


bool MapManager::checksum() {
    bool valid = true;

    if (map->total_slots < 0 || map->total_slots > SLOTS_MAX_SIZE) {
        std::cerr << "[MM] Error: checksum: total_slots is out of range: " << map->total_slots << "\n";
        return false;
    }

    std::unordered_set<int> seen_ids;
    for (int i=0; i<map->total_slots; ++i) {
        const Slot& slot = map->slots[i];
        if (seen_ids.count(slot.slot_id)) {
            std::cerr << "[MM] Error: checksum: Duplicate slot_id found: " << slot.slot_id << "\n";
            valid = false;
        }
        seen_ids.insert(slot.slot_id);
    }

    return valid;
}


bool MapManager::update_slot(int slot_id, const SlotState& state) {
    for (int i=0; i<map->total_slots; ++i) {
        if (map->slots[i].slot_id == slot_id) {
            map->slots[i].state = state;
            save_map_data();

            // Map Update Sync for Device
            pthread_mutex_lock(&map->mutex_map_dev);
            map->flag_map_dev = true;
            pthread_cond_signal(&map->cv_map_dev);
            std::cout << "[MAP_MGR][DEBUG] dev flag: " << map->flag_map_dev << "\n";
            pthread_mutex_unlock(&map->mutex_map_dev);

            // Map Update Sync for Client
            pthread_mutex_lock(&map->mutex_map_clt);
            map->flag_map_clt = true;
            pthread_cond_signal(&map->cv_map_clt);
            std::cout << "[MAP_MGR][DEBUG] clt flag: " << map->flag_map_clt << "\n";
            pthread_mutex_unlock(&map->mutex_map_clt);

            // [Debug Session]
            std::cout << "[MAP_MGR][DEBUG] Slot " << map->slots[i].slot_id << ": state=" << map->slots[i].state << "\n";

            return true;
        }
    }

    std::cerr << "[MM] Warning: update: slot_id " << slot_id << " not found.\n";
    return false;
}


void MapManager::sort_slots() {
    std::sort(map->slots, map->slots + map->total_slots,
        [](const Slot& a, const Slot& b) {
            return a.slot_id < b.slot_id;
        });
}


bool MapManager::save_map_data() {
    if (!checksum()) {
        std::cerr << "[MM] Error: save: Failed to save by checksum\n";
        return false;
    }
    sort_slots();

    std::ofstream file(file_path);
    if (!file.is_open()) return false;

    file << "{\n";
    file << "  \"parking_lot_id\": \"" << map->parking_lot_id << "\",\n";
    file << "  \"total_slots\": " << map->total_slots << ",\n";
    file << "  \"slots\": [\n";

    for (size_t i = 0; i < map->total_slots; ++i) {
        const auto& s = map->slots[i];
        file << "    {\n";
        file << "      \"slot_id\": " << s.slot_id << ",\n";
        file << "      \"state\": " << static_cast<int>(s.state) << ",\n";
        file << "      \"poly\": [";
        for (size_t j=0; j<4; ++j) {
            file << "[" << s.poly[j].x << ", " << s.poly[j].y << "]";
            if (j < 3) file << ", ";
        }
        file << "]\n";
        file << "    }";
        if (i != (map->total_slots - 1)) file << ",";
        file << "\n";
    }

    file << "  ]\n";
    file << "}\n";

    return true;
}

void MapManager::printMap() {
    std::cout << "[Parking Lot ID]: " << map->parking_lot_id
        << " (Total Slots: " << map->total_slots << ")\n";

    for (int i=0; i<map->total_slots; ++i) {
        const Slot& slot = map->slots[i];
        std::cout << "Slot " << slot.slot_id << ": " << static_cast<int>(slot.state) << " at ";
        for (int j=0; j<4; ++j) {
            std::cout << "[" << slot.poly[j].x << "," << slot.poly[j].y << "] ";
        }
        std::cout << "\n";
    }
}

const SharedParkingLotMap& MapManager::getMap() const {
    return *map;
}

pthread_mutex_t* MapManager::get_mutex_dev() {
    return &map->mutex_map_dev;
}

pthread_cond_t* MapManager::get_cv_dev() {
    return &map->cv_map_dev;
}

bool* MapManager::get_flag_ptr_dev() {
    return &map->flag_map_dev;
}

pthread_mutex_t* MapManager::get_mutex_clt() {
    return &map->mutex_map_clt;
}

pthread_cond_t* MapManager::get_cv_clt() {
    return &map->cv_map_clt;
}

bool* MapManager::get_flag_ptr_clt() {
    return &map->flag_map_clt;
}



void MapManager::destroyShm() {
    pthread_mutex_destroy(&map->mutex_map_dev);
    pthread_mutex_destroy(&map->mutex_map_clt);
    pthread_cond_destroy(&map->cv_map_dev);
    pthread_cond_destroy(&map->cv_map_clt);
    sem_destroy(&map->sem_mutex);
    shm_unlink(shm_name.c_str());
}

