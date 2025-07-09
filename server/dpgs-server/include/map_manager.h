#ifndef __MAP_MANAGER_H__
#define __MAP_MANAGER_H__

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <semaphore.h>


constexpr int LOT_NAME_SIZE     = 32;
constexpr int SLOTS_MAX_SIZE    = 29;

enum SlotState {
    EMPTY       = 0,
    OCCUPIED    = 1,
    EXITING     = 2,
    UNKNOWN     = 3
};

struct Slot {
    int         slot_id;
    SlotState   state;
    cv::Point   poly[4];
};

struct SharedParkingLotMap {
    char    parking_lot_id[LOT_NAME_SIZE];
    int     total_slots;
    Slot    slots[SLOTS_MAX_SIZE];

    sem_t   sem_mutex;
};



class MapManager {
 public:
    MapManager(const std::string& _file_path);
    ~MapManager();

    bool initialize();
    bool initialize_as_new();
    void destroyShm();

    void printMap();
    const SharedParkingLotMap& getMap() const;

//    bool insert_slot(int slot_id, const std::string& state, const std::vector<cv::Point>& poly);
    bool update_slot(int slot_id, const SlotState& state);
//    bool delete_slot(int slot_id);
    bool save_map_data();

 private:
    std::string file_path;
    std::string shm_name;
    int         shm_fd          = -1;
    void*       shm_ptr         = nullptr;
    size_t      shm_total_size  = 0;

    SharedParkingLotMap* map = nullptr;

//    bool newDB();
    bool load_map_data();
    bool checksum();
    void sort_slots();

};


#endif // __MAP_MANAGER_H__
