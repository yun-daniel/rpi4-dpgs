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
    , pmap(nullptr)
{
    ui->setupUi(this);

    initialize_slot_names();
    setup_pmap();
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
}

void Page2nd::handle_logout_button_clicked()
{
    QMessageBox::information(this, "관리자 모드", "로그인 화면으로 전환합니다.");
    release_stream();
    send_logout_and_close();
    stacked->setCurrentIndex(0);
}

void Page2nd::handle_exit_button_clicked()
{
    QMessageBox::information(this, "관리자 모드", "프로그램을 종료합니다.");
    release_stream();
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
    QString cert = QCoreApplication::applicationDirPath() + "/../../certs/server2.crt";
    QFile in("certs/server2.crt");
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

void Page2nd::setup_pmap()
{

    pmap = new ParkingMapWidget();
    pmap->setFixedSize(1050, 640);

    QGraphicsScene* scene = new QGraphicsScene(this);
    QGraphicsProxyWidget* proxy = scene->addWidget(pmap);

    ui->mini_map_view->setFrameShape(QFrame::NoFrame);
    ui->mini_map_view->setContentsMargins(0, 0, 0, 0);
    ui->mini_map_view->setScene(scene);
    ui->mini_map_view->setRenderHint(QPainter::Antialiasing);
    ui->mini_map_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->mini_map_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->mini_map_view->fitInView(proxy->boundingRect(), Qt::KeepAspectRatio);

    scene->setSceneRect(proxy->boundingRect());

    connect(pmap->get_cctv1_button(), &QPushButton::clicked, this, &Page2nd::handle_cctv1_button_clicked);
    connect(pmap->get_cctv2_button(), &QPushButton::clicked, this, &Page2nd::handle_cctv2_button_clicked);
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

    ui->floor_table_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->floor_table_view->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->floor_table_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->floor_table_view->setAlternatingRowColors(true);
    ui->floor_table_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->floor_table_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->floor_table_view->horizontalHeader()->setStretchLastSection(true);
    ui->floor_table_view->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->floor_table_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->floor_table_view->setColumnWidth(0, 100);
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
    if (cctv2Timer) {
        cctv2Timer->stop();
        delete cctv2Timer;
        cctv2Timer = nullptr;
    }
}

void Page2nd::append_log_message(const QString &message)
{
    QString fullMessage = QString("[%1] %2").arg(QDateTime::currentDateTime().toString("hh:mm:ss"), message);
    ui->logList->addItem(new QListWidgetItem(fullMessage));
    ui->logList->scrollToBottom();

    if (ui->logList->count() > 100)
    {
        delete ui->logList->takeItem(0);
    }
}

void Page2nd::read_map_data()
{
    static QByteArray buffer;

    buffer.append(socket->readAll());

    while (buffer.size() >= sizeof(SharedParkingLotMap)) {
        SharedParkingLotMap mapData;

        memcpy(&mapData, buffer.constData(), sizeof(SharedParkingLotMap));
        for(int i=0;i<SLOTS_MAX_SIZE;i++)
        {
            qDebug() << "ID : "<< mapData.slotlist[i].slot_id << "State : "  <<mapData.slotlist[i].state;
        }
        buffer.remove(0, sizeof(SharedParkingLotMap));
        update_parking_map(mapData);
    }
}

void Page2nd::initialize_slot_names()
{
    slotNameMap[4] = "B1";
    slotNameMap[5] = "B2";
    slotNameMap[6] = "B3";

    floorSlotMap =
    {
        {"B1F", {4, 5, 6}}
    };
}

void Page2nd::update_parking_map(const SharedParkingLotMap &map)
{
    int availableCount = 0;

    QList<int> b1fSlots = floorSlotMap.value("B1F");

    for (int i = 0; i < SLOTS_MAX_SIZE; ++i)
    {
        const Slot &slot = map.slotlist[i];

        pmap->update_slot_state(slot.slot_id, slot.state);

        if (slot.slot_id != 0 && previousSlotStates[i] != slot.state)
        {
            QString stateStr;
            switch (slot.state)
            {
            case EMPTY:    stateStr = "주차 가능"; break;
            case OCCUPIED: stateStr = "주차 불가능"; break;
            case EXITING:  stateStr = "출차 예정 감지"; break;
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

