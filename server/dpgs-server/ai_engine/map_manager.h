#ifndef __MAP_MANAGER_H__
#define __MAP_MANAGER_H__

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>


struct Slot {
    int slot_id;
    std::string state;
    std::vector<cv::Point> poly;
};

struct ParkingLotMap {
    std::string parking_lot_id;
    int total_slots;
    std::vector<Slot> slots;
};


class MapManager {
 public:
    MapManager();
    MapManager(const std::string& _path);

    void printMap();
    const ParkingLotMap& getMap() const;

    bool insert_slot(int slot_id, const std::string& state, const std::vector<cv::Point>& poly);
    bool update_slot(int slot_id, const std::string& state);
    bool delete_slot(int slot_id);
    bool save();

 private:
    std::string file_path;
    ParkingLotMap map;

    bool newDB();
    bool load();
    bool checksum();
    void sort_slots();

};


#endif // __MAP_MANAGER_H__
