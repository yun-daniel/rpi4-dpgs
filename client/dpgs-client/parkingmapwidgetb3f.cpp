#include "parkingmapwidgetb3f.h"
#include "ui_parkingmapwidgetb3f.h"

ParkingMapWidgetB3F::ParkingMapWidgetB3F(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ParkingMapWidgetB3F)
{
    ui->setupUi(this);
}

ParkingMapWidgetB3F::~ParkingMapWidgetB3F()
{
    delete ui;
}
