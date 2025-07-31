#include "parkingmapwidgetb2f.h"
#include "ui_parkingmapwidgetb2f.h"

ParkingMapWidgetB2F::ParkingMapWidgetB2F(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ParkingMapWidgetB2F)
{
    ui->setupUi(this);
}

ParkingMapWidgetB2F::~ParkingMapWidgetB2F()
{
    delete ui;
}
