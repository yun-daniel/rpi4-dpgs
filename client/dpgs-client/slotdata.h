#ifndef SLOTDATA_H
#define SLOTDATA_H

constexpr int LOT_NAME_SIZE = 32;
constexpr int SLOTS_MAX_SIZE = 29;

enum SlotState
{
    EMPTY       = 0,
    OCCUPIED    = 1,
    EXITING     = 2,
    UNKNOWN     = 3
};

struct Point
{
    int x;
    int y;
};

struct Slot
{
    int         slot_id;
    SlotState   state;
    Point       poly[4];
};

struct SharedParkingLotMap
{
    Slot    slotlist[SLOTS_MAX_SIZE];
};

#endif
