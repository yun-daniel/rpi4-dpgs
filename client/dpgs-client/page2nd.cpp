#include "page2nd.h"
#include "ui_page2nd.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDateTime>
#include <QFile>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QMessageBox>
#include <QPixmap>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QSslSocket>

Page2nd::Page2nd(QStackedWidget *parent_stacked, QSslSocket *sharedSocket, QWidget *parent)
    : QWidget{parent}
    , ui(new Ui::Page2nd)
    , stacked(parent_stacked)
    , socket(sharedSocket)
    , timer(nullptr)
{
    ui->setupUi(this);
    ui->log_tab_widget->setCurrentIndex(0);

    initialize_slot_names();
    setup_all_maps();
    create_dummy_log();
    setup_floor_table();
    previousSlotStates.fill(UNKNOWN, SLOTS_MAX_SIZE);
    setup_connections();
}

Page2nd::~Page2nd()
{
    release_stream();
    delete ui;
}

void Page2nd::setup_connections()
{
    connect(ui->logout_button, &QPushButton::clicked, this, &Page2nd::handle_logout_button_clicked);
    connect(ui->exit_button, &QPushButton::clicked, this, &Page2nd::handle_exit_button_clicked);
    connect(stacked, &QStackedWidget::currentChanged, this, &Page2nd::handle_page_changed);
}

void Page2nd::handle_page_changed(int index)
{
    disconnect(socket, &QSslSocket::readyRead, this, &Page2nd::read_map_data);

    if (index == 1)
    {
        connect(socket, &QSslSocket::readyRead, this, &Page2nd::read_map_data);
        show_no_signal();
    }
}

void Page2nd::handle_cctv1_button_clicked()
{
    cctv2_mode = false;

    if (socket && socket->state() == QAbstractSocket::ConnectedState)
    {
        QByteArray cmd(1, '\0');
        cmd[0] = '1';
        socket->write(cmd);
        socket->flush();
    }

    if (!streamStarted)
    {
        setup_gstreamer();
    }

    stop_cctv2_signal_timer();

    if (timer && !timer->isActive())
    {
        timer->start(33);
    }

    qDebug() << "[CCTV 1] 스트리밍 화면 표시";

    toggle_camera1_cone();
}

void Page2nd::handle_cctv2_button_clicked()
{
    cctv2_mode = true;

    if (socket && socket->state() == QAbstractSocket::ConnectedState)
    {
        QByteArray cmd(1, '\0');
        cmd[0] = '2';
        socket->write(cmd);
        socket->flush();
    }

    if (!streamStarted)
    {
        setup_gstreamer();
    }

    if (timer && timer->isActive())
    {
        timer->stop();
    }

    start_cctv2_signal_timer();

    qDebug() << "[CCTV 2] No Signal 표시";

    if (camera1Cone)
    {
        miniMapScene->removeItem(camera1Cone);
        delete camera1Cone;
        camera1Cone = nullptr;
    }

    activeCameraId = 2;
}

void Page2nd::handle_logout_button_clicked()
{
    QMessageBox::information(this, "관리자 모드", "로그인 화면으로 전환합니다.");
    release_stream();
    clear_camera_cones();
    activeCameraId = 0;
    send_logout_and_close();
    stacked->setCurrentIndex(0);
}

void Page2nd::handle_exit_button_clicked()
{
    QMessageBox::information(this, "관리자 모드", "프로그램을 종료합니다.");
    release_stream();
    clear_camera_cones();
    activeCameraId = 0;
    send_logout_and_close();
    qApp->exit();
}

void Page2nd::send_logout_and_close()
{
    if (socket && socket->state() == QAbstractSocket::ConnectedState)
    {
        QByteArray cmd(1, '\0');
        cmd[0] = '0';
        socket->write(cmd);
        socket->flush();
        socket->disconnectFromHost();
        if (socket->state() != QAbstractSocket::UnconnectedState)
        {
            socket->waitForDisconnected(1000);
        }
    }
}

void Page2nd::release_stream()
{
    if (timer)
    {
        timer->stop();
        delete timer;
        timer = nullptr;
    }

    if (cctvThread)
    {
        cctvThread->stop_streaming();
        cctvThread->deleteLater();
        cctvThread = nullptr;
    }

    streamStarted = false;

    qDebug() << "[CCTV] 스트리밍 중단";
}

void Page2nd::setup_gstreamer()
{
    if (cctvThread && streamStarted) return;

    QString uri = "rtsps://192.168.0.67:8555/stream";
    QString cert = QCoreApplication::applicationDirPath() + "/../../certs/server.crt";
    QFile in("certs/server.crt");
    QFile out(cert);
    if (in.open(QIODevice::ReadOnly) && out.open(QIODevice::WriteOnly))
    {
        out.write(in.readAll());
        out.close();
    }

    cctvThread = new CCTVStreamThread(cctv2_mode, this);

    if (!cctvThread->init(uri, cert))
    {
        QMessageBox::critical(this, "CCTV 연결 오류", "CCTV 스트리밍을 시작할 수 없습니다.");
        delete cctvThread;
        cctvThread = nullptr;

        show_no_signal();

        return;
    }

    connect(cctvThread, &CCTVStreamThread::frame_ready, this, [=](const QImage &img) {
        if (cctv2_mode) return;

        QPixmap pixmap = QPixmap::fromImage(img).scaled(
            ui->label_cctv->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label_cctv->setPixmap(pixmap);
    });

    cctvThread->start();
    streamStarted = true;
    stop_cctv2_signal_timer();

    qDebug() << "[GStreamer] 스트리밍 시작 완료";
}

void Page2nd::setup_all_maps()
{
    mapB1F = new ParkingMapWidget();
    mapB2F = new ParkingMapWidgetB2F();
    mapB3F = new ParkingMapWidgetB3F();

    connect(mapB1F->get_cctv1_button(), &QPushButton::clicked, this, &Page2nd::handle_cctv1_button_clicked);
    connect(mapB1F->get_cctv2_button(), &QPushButton::clicked, this, &Page2nd::handle_cctv2_button_clicked);

    miniMapScene = new QGraphicsScene(this);
    ui->mini_map_view->setScene(miniMapScene);
    ui->mini_map_view->setRenderHint(QPainter::Antialiasing);

    mapB1FProxy = miniMapScene->addWidget(mapB1F);
    mapB2FProxy = miniMapScene->addWidget(mapB2F);
    mapB3FProxy = miniMapScene->addWidget(mapB3F);

    mapB1FProxy->setVisible(true);
    mapB2FProxy->setVisible(false);
    mapB3FProxy->setVisible(false);

    miniMapScene->setSceneRect(mapB1FProxy->boundingRect());
    ui->mini_map_view->setFrameShape(QFrame::NoFrame);
    ui->mini_map_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->mini_map_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->mini_map_view->fitInView(mapB1FProxy->boundingRect(), Qt::KeepAspectRatio);

    currentFloor = "B1F";
}

void Page2nd::create_dummy_log()
{
    ui->loglist_b2->addItem(new QListWidgetItem("[10:40:20] [B4] → 비어 있음"));
    ui->loglist_b2->addItem(new QListWidgetItem("[11:56:20] [E2] → 비어 있음"));
    ui->loglist_b2->addItem(new QListWidgetItem("[11:56:20] [B2] → 비어 있음"));
    ui->loglist_b2->addItem(new QListWidgetItem("[11:56:20] [A2] → 비어 있음"));
    ui->loglist_b2->addItem(new QListWidgetItem("[12:00:01] [A4] → 사용 중"));
    ui->loglist_b2->addItem(new QListWidgetItem("[12:15:05] [E3] → 출차 예정"));
    ui->loglist_b2->addItem(new QListWidgetItem("[12:15:20] [E3] → 비어 있음"));

    ui->loglist_b3->addItem(new QListWidgetItem("[08:58:02] [A1] → 출차 예정"));
    ui->loglist_b3->addItem(new QListWidgetItem("[08:58:12] [D4] → 출차 예정"));
    ui->loglist_b3->addItem(new QListWidgetItem("[08:58:15] [A1] → 비어 있음"));
    ui->loglist_b3->addItem(new QListWidgetItem("[08:58:30] [D4] → 비어 있음"));
    ui->loglist_b3->addItem(new QListWidgetItem("[10:37:08] [B3] → 사용 중"));
    ui->loglist_b3->addItem(new QListWidgetItem("[10:50:10] [B1] → 사용 중"));
    ui->loglist_b3->addItem(new QListWidgetItem("[11:47:02] [D5] → 출차 예정"));
    ui->loglist_b3->addItem(new QListWidgetItem("[11:47:22] [D5] → 비어 있음"));
}

void Page2nd::switch_floor_map(const QString &floor)
{
    if (currentFloor == floor)
        return;

    clear_camera_cones();
    currentFloor = floor;

    if (floor == "B1F")
    {
        mapB1FProxy->setVisible(true);
        mapB2FProxy->setVisible(false);
        mapB3FProxy->setVisible(false);
        miniMapScene->setSceneRect(mapB1FProxy->boundingRect());
        ui->mini_map_view->setFrameShape(QFrame::NoFrame);
        ui->mini_map_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui->mini_map_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui->mini_map_view->fitInView(mapB1FProxy->boundingRect(), Qt::KeepAspectRatio);

        if (activeCameraId == 1)
            toggle_camera1_cone();
    }
    else if (floor == "B2F")
    {
        mapB1FProxy->setVisible(false);
        mapB2FProxy->setVisible(true);
        mapB3FProxy->setVisible(false);
        miniMapScene->setSceneRect(mapB2FProxy->boundingRect());
        ui->mini_map_view->setFrameShape(QFrame::NoFrame);
        ui->mini_map_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui->mini_map_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui->mini_map_view->fitInView(mapB2FProxy->boundingRect(), Qt::KeepAspectRatio);
    }
    else if (floor == "B3F")
    {
        mapB1FProxy->setVisible(false);
        mapB2FProxy->setVisible(false);
        mapB3FProxy->setVisible(true);
        miniMapScene->setSceneRect(mapB3FProxy->boundingRect());
        ui->mini_map_view->setFrameShape(QFrame::NoFrame);
        ui->mini_map_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui->mini_map_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui->mini_map_view->fitInView(mapB3FProxy->boundingRect(), Qt::KeepAspectRatio);
    }
}

void Page2nd::setup_floor_table()
{
    floorModel = new QStandardItemModel(this);
    floorModel->setHorizontalHeaderLabels({"층 수", "주차 가능 공간 수"});

    QMap<QString, QString> initialSpaces = {
        {"B1F", "0대"},   // 실시간 반영 (처음은 0)
        {"B2F", "5대"},   // 하드코딩
        {"B3F", "3대"}    // 하드코딩
    };

    for (auto it = initialSpaces.constBegin(); it != initialSpaces.constEnd(); ++it)
    {
        const QString &floor = it.key();
        const QString &space = it.value();

        QStandardItem *floorItem = new QStandardItem(floor);
        QStandardItem *spaceItem = new QStandardItem(space);

        floorItem->setTextAlignment(Qt::AlignCenter);
        spaceItem->setTextAlignment(Qt::AlignCenter);

        floorModel->appendRow({floorItem, spaceItem});
    }

    ui->floor_table_view->setModel(floorModel);
    ui->floor_table_view->setMouseTracking(true);

    ui->floor_table_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->floor_table_view->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->floor_table_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->floor_table_view->setAlternatingRowColors(true);
    ui->floor_table_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->floor_table_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->floor_table_view->horizontalHeader()->setStretchLastSection(true);
    ui->floor_table_view->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->floor_table_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->floor_table_view->setColumnWidth(0, 200);
    ui->floor_table_view->verticalHeader()->setVisible(false);

    int totalHeight = ui->floor_table_view->viewport()->height();
    int rowCount = floorModel->rowCount();

    if (rowCount > 0)
    {
        int baseRowHeight = totalHeight / rowCount;
        int remain = totalHeight - (baseRowHeight * rowCount);

        for (int i = 0; i < rowCount; ++i)
        {
            int rowHeight = baseRowHeight + ((i == rowCount - 1) ? remain : 0);
            ui->floor_table_view->setRowHeight(i, rowHeight);
        }
    }

    connect(ui->floor_table_view, &QTableView::doubleClicked, this, &Page2nd::handle_floor_clicked);
}

void Page2nd::handle_floor_clicked(const QModelIndex &index)
{
    QString floor = floorModel->item(index.row(), 0)->text();
    if (ui->label_floor) ui->label_floor->setText(floor);

    switch_floor_map(floor);

    if (floor == "B1F") {
        ui->log_tab_widget->setCurrentIndex(0);
    }
    else if (floor == "B2F") {
        ui->log_tab_widget->setCurrentIndex(1);
    }
    else if (floor == "B3F") {
        ui->log_tab_widget->setCurrentIndex(2);
    }
}

void Page2nd::show_no_signal()
{
    ui->label_cctv->clear();

    QPixmap pixmap(ui->label_cctv->size());
    pixmap.fill(Qt::black);

    QPainter painter(&pixmap);
    QFont font;
    font.setPointSize(20);
    font.setItalic(true);
    font.setBold(true);
    painter.setPen(Qt::white);
    painter.setFont(font);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, "No Signal");
    painter.end();

    ui->label_cctv->setPixmap(pixmap);
}

void Page2nd::show_cctv2_signal()
{
    ui->label_cctv->clear();

    QPixmap pixmap(ui->label_cctv->size());
    pixmap.fill(Qt::black);

    QPainter painter(&pixmap);
    painter.setPen(Qt::white);

    QFont fontTitle;
    fontTitle.setPointSize(20);
    fontTitle.setBold(true);
    fontTitle.setItalic(true);
    painter.setFont(fontTitle);

    QRect titleRect(0, pixmap.height() / 2 - 30, pixmap.width(), 40);
    painter.drawText(titleRect, Qt::AlignCenter, "[CCTV 2] No Signal");

    QFont fontTime;
    fontTime.setPointSize(14);
    fontTime.setItalic(true);
    painter.setFont(fontTime);

    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QRect timeRect(0, pixmap.height() / 2 + 5, pixmap.width(), 40);
    painter.drawText(timeRect, Qt::AlignCenter, currentTime);

    painter.end();

    ui->label_cctv->setPixmap(pixmap);
}

void Page2nd::start_cctv2_signal_timer()
{
    if (!cctv2Timer)
    {
        cctv2Timer = new QTimer(this);
        connect(cctv2Timer, &QTimer::timeout, this, &Page2nd::show_cctv2_signal);
    }

    cctv2Timer->start(1000);
    show_cctv2_signal();
}

void Page2nd::stop_cctv2_signal_timer()
{
    if (cctv2Timer)
    {
        cctv2Timer->stop();
        delete cctv2Timer;
        cctv2Timer = nullptr;
    }
}

void Page2nd::append_log_message(const QString &message)
{
    QString fullMessage = QString("[%1] %2").arg(QDateTime::currentDateTime().toString("hh:mm:ss"), message);
    QListWidget *target_log_list = ui->loglist_b1;
    target_log_list->addItem(new QListWidgetItem(fullMessage));
    target_log_list->scrollToBottom();
    if (target_log_list->count() > 100)
    {
        delete target_log_list->takeItem(0);
    }
}

void Page2nd::read_map_data()
{
    static QByteArray buffer;

    buffer.append(socket->readAll());

    while (buffer.size() >= sizeof(SharedParkingLotMap))
    {
        SharedParkingLotMap mapData;

        memcpy(&mapData, buffer.constData(), sizeof(SharedParkingLotMap));
        for (int i = 0; i < SLOTS_MAX_SIZE; ++i)
        {
            int id = mapData.slotlist[i].slot_id;
            if (id >= 22 && id <= 26)
            {
                qDebug() << "ID : " << id << "State : " << mapData.slotlist[i].state;
            }
        }
        buffer.remove(0, sizeof(SharedParkingLotMap));
        update_parking_map(mapData);
    }
}

void Page2nd::initialize_slot_names()
{
    for (int i = 0; i < 29; ++i)
    {
        QString name;

        if (i >= 0 && i <= 3)       name = QString("A%1").arg(i + 1);
        else if (i >= 4 && i <= 10) name = QString("B%1").arg(i - 3);
        else if (i >= 11 && i <= 14)name = QString("C%1").arg(i - 10);
        else if (i >= 15 && i <= 21)name = QString("D%1").arg(i - 14);
        else if (i >= 22 && i <= 28)name = QString("E%1").arg(i - 21);

        slotNameMap[i] = name;
    }

    floorSlotMap = {{"B1F", {22, 23, 24, 25, 26}}};
}

void Page2nd::update_parking_map(const SharedParkingLotMap &map)
{
    int availableCount = 0;

    QList<int> b1fSlots = floorSlotMap.value("B1F");

    for (int i = 0; i < SLOTS_MAX_SIZE; ++i)
    {
        const Slot &slot = map.slotlist[i];

        if (!b1fSlots.contains(slot.slot_id))
            continue;

        mapB1F->update_slot_state(slot.slot_id, slot.state);

        if (previousSlotStates[i] != slot.state)
        {
            QString stateStr;
            switch (slot.state)
            {
            case EMPTY:    stateStr = "비어 있음"; break;
            case OCCUPIED: stateStr = "사용 중"; break;
            case EXITING:  stateStr = "출차 예정"; break;
            case UNKNOWN:  stateStr = "UNKNOWN"; break;
            default:       stateStr = "???"; break;
            }

            QString slotName = slotNameMap.value(slot.slot_id, QString("슬롯%1").arg(slot.slot_id));
            append_log_message(QString("[%1] → %2").arg(slotName, stateStr));
            previousSlotStates[i] = slot.state;
        }

        if (b1fSlots.contains(slot.slot_id) && slot.state == EMPTY)
        {
            availableCount++;
        }
    }

    for (int row = 0; row < floorModel->rowCount(); ++row)
    {
        if (floorModel->item(row, 0)->text() == "B1F")
        {
            QStandardItem *item = floorModel->item(row, 1);
            if (item)
            {
                item->setText(QString("%1대").arg(availableCount));
                item->setTextAlignment(Qt::AlignCenter);
            }
            break;
        }
    }
}

void Page2nd::toggle_camera1_cone()
{
    if (camera1Cone)
    {
        miniMapScene->removeItem(camera1Cone);
        delete camera1Cone;
        camera1Cone = nullptr;
    }

    QPointF cameraPos(655, 580);

    const int height = 95;
    const int width = 190;

    QPointF leftTop(cameraPos.x() - width, cameraPos.y() - height);
    QPointF rightTop(cameraPos.x() + width, cameraPos.y() - height);

    QPainterPath path;
    path.moveTo(cameraPos);
    path.lineTo(leftTop);
    path.lineTo(rightTop);
    path.closeSubpath();

    QLinearGradient gradient(cameraPos, QPointF(cameraPos.x(), cameraPos.y() - height));
    gradient.setColorAt(0.0, QColor(0, 255, 255, 170));
    gradient.setColorAt(1.0, QColor(0, 255, 255, 5));

    camera1Cone = new QGraphicsPathItem(path);
    camera1Cone->setBrush(gradient);
    camera1Cone->setPen(Qt::NoPen);

    miniMapScene->addItem(camera1Cone);
    activeCameraId = 1;
}

void Page2nd::clear_camera_cones()
{
    if (camera1Cone)
    {
        miniMapScene->removeItem(camera1Cone);
        delete camera1Cone;
        camera1Cone = nullptr;
    }
}

