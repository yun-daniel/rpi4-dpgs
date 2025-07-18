#ifndef __MAP_MANAGER_H__
#define __MAP_MANAGER_H__

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <semaphore.h>
#include <condition_variable>
#include <atomic>


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

    // Device Manager Sync
    pthread_mutex_t mutex_map_dev;
    pthread_cond_t  cv_map_dev;
    bool            flag_map_dev;

    // Client Manager Sync
    pthread_mutex_t mutex_map_clt;
    pthread_cond_t  cv_map_clt;
    bool            flag_map_clt;
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

    bool update_slot(int slot_id, const SlotState& state);
    bool save_map_data();

    // --- Map Update Sync ---
    // Device Manager
    pthread_mutex_t*    get_mutex_dev();
    pthread_cond_t*     get_cv_dev();
    bool*               get_flag_ptr_dev();
    // Client Manager
    pthread_mutex_t*    get_mutex_clt();
    pthread_cond_t*     get_cv_clt();
    bool*               get_flag_ptr_clt();


 private:
    std::string file_path;
    std::string shm_name;
    int         shm_fd          = -1;
    void*       shm_ptr         = nullptr;
    size_t      shm_total_size  = 0;

    SharedParkingLotMap* map = nullptr;

    bool load_map_data();
    bool checksum();
    void sort_slots();

};


#endif // __MAP_MANAGER_H__
