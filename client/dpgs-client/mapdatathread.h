#ifndef MAPDATATHREAD_H
#define MAPDATATHREAD_H

#include <QThread>
#include <QTcpSocket>

// 서버와 통일된 슬롯 상태 enum
enum SlotState {
    EMPTY = 0,
    OCCUPIED = 1,
    EXITING = 2,
    UNKNOWN = 3
};

// 클라이언트에서 사용하는 최소 슬롯 정보 구조체
struct SlotInfo {
    int slot_id;
    SlotState state;
};

// 서버에서 보내는 Slot 구조체의 크기 (slot_id + state + poly[4])
constexpr int SLOT_STRUCT_SIZE = 40;
constexpr int SLOTS_MAX_SIZE = 29;
constexpr int SLOT_TOTAL_SIZE = SLOT_STRUCT_SIZE * SLOTS_MAX_SIZE;

class MapDataThread : public QThread
{
    Q_OBJECT

public:
    explicit MapDataThread(QTcpSocket *socket, QObject *parent = nullptr);
    ~MapDataThread();

signals:
    void newSlotDataReceived(const QList<SlotInfo> &slots);

protected:
    void run() override;

private:
    QTcpSocket *socket;
};

#endif
