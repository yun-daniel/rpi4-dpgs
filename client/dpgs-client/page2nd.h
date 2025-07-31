#ifndef PAGE2ND_H
#define PAGE2ND_H

#include "cctvstreamthread.h"
#include "parkingmapwidget.h"
#include "parkingmapwidgetb2f.h"
#include "parkingmapwidgetb3f.h"
#include "slotdata.h"

#include <QWidget>
#include <QTimer>
#include <QStandardItemModel>
#include <QSslSocket>

#include <QGraphicsScene>
#include <QGraphicsPathItem>
#include <QPainterPath>

QT_BEGIN_NAMESPACE
namespace Ui { class Page2nd; }
QT_END_NAMESPACE

class QStackedWidget;
class QStandardItemModel;
class QSslSocket;
class ParkingMapWidget;

class Page2nd : public QWidget
{
    Q_OBJECT

public:
    explicit Page2nd(QStackedWidget *parent_stacked, QSslSocket *sharedSocket, QWidget *parent = nullptr);
    ~Page2nd();

private slots:
    void handle_cctv1_button_clicked();
    void handle_cctv2_button_clicked();
    void handle_logout_button_clicked();
    void handle_exit_button_clicked();
    void handle_page_changed(int index);
    void handle_floor_clicked(const QModelIndex &index);

    void read_map_data();

    void toggle_camera1_cone();
    void toggle_camera2_cone();
    void clear_camera_cones();

private:
    void show_no_signal();
    void show_cctv2_signal();
    void start_cctv2_signal_timer();
    void stop_cctv2_signal_timer();

    void setup_connections();
    void setup_gstreamer();
    void setup_all_maps();
    void setup_floor_table();

    void initialize_slot_names();
    void update_parking_map(const SharedParkingLotMap &map);
    void append_log_message(const QString &message);
    void switch_floor_map(const QString &floor);
    void release_stream();
    void send_logout_and_close();

private:
    Ui::Page2nd *ui;
    QStackedWidget *stacked;
    QSslSocket *socket;

    QTimer *timer;
    QTimer *cctv2Timer = nullptr;

    ParkingMapWidget *pmap;
    QStandardItemModel *floorModel;

    CCTVStreamThread *cctvThread = nullptr;

    QVector<SlotState> previousSlotStates;
    QMap<int, QString> slotNameMap;
    QMap<QString, QList<int>> floorSlotMap;

    QGraphicsScene *miniMapScene = nullptr;
    QGraphicsPathItem *camera1Cone = nullptr;
    QGraphicsPathItem *camera2Cone = nullptr;

    QGraphicsProxyWidget *mapB1FProxy = nullptr;
    QGraphicsProxyWidget *mapB2FProxy = nullptr;
    QGraphicsProxyWidget *mapB3FProxy = nullptr;

    ParkingMapWidget *mapB1F = nullptr;
    ParkingMapWidgetB2F *mapB2F = nullptr;
    ParkingMapWidgetB3F *mapB3F = nullptr;
    QString currentFloor = "B1F";

private:
    bool streamStarted = false;
    bool cctv2_mode = false;
    int activeCameraId = 0;
};

#endif
