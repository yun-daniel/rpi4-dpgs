#include "mapdatathread.h"
#include <QDebug>

MapDataThread::MapDataThread(QTcpSocket *sharedSocket, QObject *parent)
    : QThread(parent), socket(sharedSocket)
{
    qDebug() << "[MapDataThread] 스레드 생성";
}

MapDataThread::~MapDataThread()
{
    requestInterruption();
    wait();
}

void MapDataThread::run()
{
    while (!isInterruptionRequested()) {
        if (socket && socket->waitForReadyRead(100)) {
            QByteArray data = socket->read(SLOT_TOTAL_SIZE);

            // 수신 크기 확인
            if (data.size() != SLOT_TOTAL_SIZE) {
                qWarning() << "[MapDataThread] 수신 데이터 크기 불일치:" << data.size();
                continue;
            }

            QList<SlotInfo> result;
            const char *raw = data.constData();

            for (int i = 0; i < SLOTS_MAX_SIZE; ++i) {
                int slot_id = *reinterpret_cast<const int *>(raw);
                int state   = *reinterpret_cast<const int *>(raw + 4);
                raw += SLOT_STRUCT_SIZE;

                SlotInfo s;
                s.slot_id = slot_id;
                s.state = static_cast<SlotState>(state);
                result.append(s);
            }

            emit newSlotDataReceived(result);
        }

        msleep(50);
    }

    qDebug() << "[MapDataThread] 스레드 종료";
}
