// slotinfo.h
#ifndef SLOTINFO_H
#define SLOTINFO_H

enum SlotState {
    EMPTY = 0,
    OCCUPIED = 1,
    EXITING = 2,
    UNKNOWN = 3
};

struct SlotInfo {
    int slot_id;
    SlotState state;
};

#endif // SLOTINFO_H
