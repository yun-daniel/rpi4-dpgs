#ifndef PARKINGMAPWIDGET_H
#define PARKINGMAPWIDGET_H

#include "slotdata.h"

#include <QWidget>
#include <QPaintEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class ParkingMapWidget; }
QT_END_NAMESPACE

class QPushButton;

class ParkingMapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ParkingMapWidget(QWidget *parent = nullptr);
    ~ParkingMapWidget();

    QPushButton *get_cctv1_button() {return cctv1_button;}
    QPushButton *get_cctv2_button() {return cctv2_button;}

    void updateSlotState(int slot_id, SlotState state);
private:
    Ui::ParkingMapWidget *ui;

    QPushButton *cctv1_button;
    QPushButton *cctv2_button;
    QMap<int, QPushButton*> slotButtons;
};

#endif
