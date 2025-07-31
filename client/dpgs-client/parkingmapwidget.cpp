#include "parkingmapwidget.h"
#include "ui_parkingmapwidget.h"

#include <QStyleOption>
#include <QPainter>

ParkingMapWidget::ParkingMapWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ParkingMapWidget)
{
    ui->setupUi(this);
    ui->cctv_button_1->setCheckable(true);
    ui->cctv_button_2->setCheckable(true);

    cctv1_button = ui->cctv_button_1;
    cctv2_button = ui->cctv_button_2;

    initialize_slot_buttons();

    this->setAttribute(Qt::WA_StyledBackground, true);
    this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
}

ParkingMapWidget::~ParkingMapWidget()
{
    delete ui;
}

void ParkingMapWidget::update_slot_state(int slot_id, SlotState state)
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

void ParkingMapWidget::initialize_slot_buttons()
{
    QStringList sections = {"A", "B", "C", "D", "E"};
    QMap<QString, int> sectionStartId =
    {
        {"A", 0},
        {"B", 4},
        {"C", 11},
        {"D", 15},
        {"E", 22}
    };

    QMap<QString, int> sectionCount =
    {
        {"A", 4},
        {"B", 7},
        {"C", 4},
        {"D", 7},
        {"E", 7}
    };

    for (const QString& section : sections)
    {
        int startId = sectionStartId[section];
        int count = sectionCount[section];
        for (int i = 0; i < count; ++i)
        {
            int slot_id = startId + i;
            QString buttonName = QString("%1%2_button").arg(section).arg(i + 1);
            QPushButton* btn = findChild<QPushButton*>(buttonName);
            if (btn)
            {
                slotButtons[slot_id] = btn;
            }
            else
            {
                qWarning() << "Button not found:" << buttonName;
            }
        }
    }
}

