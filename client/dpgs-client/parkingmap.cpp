#include "parkingmap.h"
#include "ui_parkingmap.h"

parkingMap::parkingMap(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::parkingMap)
{
    ui->setupUi(this);
}

parkingMap::~parkingMap()
{
    delete ui;
}
