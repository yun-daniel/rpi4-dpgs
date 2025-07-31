#ifndef PARKINGMAPWIDGETB3F_H
#define PARKINGMAPWIDGETB3F_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class ParkingMapWidgetB3F; }
QT_END_NAMESPACE

class ParkingMapWidgetB3F : public QWidget
{
    Q_OBJECT

public:
    explicit ParkingMapWidgetB3F(QWidget *parent = nullptr);
    ~ParkingMapWidgetB3F();

private:
    Ui::ParkingMapWidgetB3F *ui;
};

#endif // PARKINGMAPWIDGETB3F_H
