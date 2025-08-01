#ifndef PARKINGMAPWIDGETB2F_H
#define PARKINGMAPWIDGETB2F_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class ParkingMapWidgetB2F; }
QT_END_NAMESPACE

class ParkingMapWidgetB2F : public QWidget
{
    Q_OBJECT

public:
    explicit ParkingMapWidgetB2F(QWidget *parent = nullptr);
    ~ParkingMapWidgetB2F();

private:
    Ui::ParkingMapWidgetB2F *ui;
};

#endif
