#ifndef PAGE2ND_H
#define PAGE2ND_H

#include "cctvstreamthread.h"
#include "parkingmapwidget.h"
#include "slotdata.h"

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QImage>
#include <QStandardItemModel>
#include <QTcpSocket>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

QT_BEGIN_NAMESPACE
namespace Ui { class Page2nd; }
QT_END_NAMESPACE

class QStackedWidget;
class QStandardItemModel;
class QTcpSocket;
class ParkingMapWidget;

class Page2nd : public QWidget
{
    Q_OBJECT

public:
    explicit Page2nd(QStackedWidget* parent_stacked, QTcpSocket *sharedSocket, QWidget *parent = nullptr);
    ~Page2nd();

private slots:
    void handle_cctv1_button_clicked();
    void handle_cctv2_button_clicked();
    void handle_logout_button_clicked();
    void handle_exit_button_clicked();
    void handle_page_changed(int index);
    void handle_floor_clicked(const QModelIndex &index);

private slots:
    void read_map_data();
    void update_parking_map(const SharedParkingLotMap &map);


private:
    void show_no_signal();
    void show_cctv2_signal();
    void start_cctv2_signal_timer();
    void stop_cctv2_signal_timer();

    void setup_connections();
    void setup_gstreamer();
    void setup_pmap();
    void setup_floor_table();

    void append_log_message(const QString &message);

    void release_stream();
    void send_logout_and_close();

    void initializeSlotNames();

private:
    Ui::Page2nd *ui;
    QStackedWidget* stacked;
    QTcpSocket *socket;

    QTimer *timer;
    QTimer* cctv2Timer = nullptr;

    ParkingMapWidget* pmap;
    QStandardItemModel* floorModel;
    QLabel* currentFloorLabel;

    CCTVStreamThread *cctvThread = nullptr;

    QByteArray buffer;
    QVector<SlotState> previousSlotStates;
    QMap<int, QString> slotNameMap;

private:
    bool streamStarted = false;
    bool cctv2_mode = false;

};

#endif
