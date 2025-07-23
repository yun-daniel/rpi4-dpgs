#include "parkingmapwidget.h"
#include "ui_parkingmapwidget.h"

#include <QStyleOption>
#include <QPainter>

ParkingMapWidget::ParkingMapWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ParkingMapWidget)
{
    ui->setupUi(this);
    cctv1_button = ui->cctv_button_1;
    cctv2_button = ui->cctv_button_2;

    slotButtons[1] = ui->B3_button;
    slotButtons[2] = ui->B2_button;
    slotButtons[3] = ui->B1_button;

    this->setAttribute(Qt::WA_StyledBackground, true);
    this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
}

ParkingMapWidget::~ParkingMapWidget()
{
    delete ui;
}

void ParkingMapWidget::updateSlotState(int slot_id, SlotState state)
{
    if (slotButtons.contains(slot_id)) {
        QPushButton *btn = slotButtons[slot_id];

        switch (state)
        {
        case EMPTY:
            btn->setStyleSheet("background-color: #32CD32;");
            break;
        case OCCUPIED:
            btn->setStyleSheet("background-color: #FF3B3B;");
            break;
        case EXITING:
            btn->setStyleSheet("background-color: #FF9900;");
            break;
        default:
            btn->setStyleSheet("background-color: gray;");
        }
    }
}
