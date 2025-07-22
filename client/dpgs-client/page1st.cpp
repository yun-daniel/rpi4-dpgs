#include "page1st.h"
#include "ui_page1st.h"

#include <QDebug>
#include <QMessageBox>
#include <QStackedWidget>
#include <QTcpSocket>
#include <QPainter>

Page1st::Page1st(QStackedWidget *parent_stacked, QTcpSocket *sharedSocket, QWidget *parent)
    : QWidget{parent}
    , ui(new Ui::Page1st)
    , stacked(parent_stacked)
    , socket(sharedSocket)
{
    ui->setupUi(this);
    ui->line_edit_pw->setEchoMode(QLineEdit::Password);
    setup_connections();
    draw_parking_labels();
}

Page1st::~Page1st()
{
    delete ui;
}

void Page1st::setup_connections()
{
    connect(ui->line_edit_id, &QLineEdit::returnPressed, ui->login_button, &QPushButton::click);
    connect(ui->line_edit_pw, &QLineEdit::returnPressed, ui->login_button, &QPushButton::click);
    connect(ui->login_button, &QPushButton::clicked, this, &Page1st::handle_login_button_clicked);
}

void Page1st::handle_login_button_clicked()
{
    const QString id = ui->line_edit_id->text().trimmed();
    const QString pw = ui->line_edit_pw->text().trimmed();

    if (id.isEmpty() || pw.isEmpty())
    {
        QMessageBox::warning(this, "입력 오류", "아이디와 비밀번호를 모두 입력해주세요.");
        return;
    }

    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->abort();
    }

    socket->connectToHost("192.168.0.32", 9999);

    if (!socket->waitForConnected(3000))
    {
        QMessageBox::critical(this, "서버 연결 실패", "서버에 연결할 수 없습니다.");
        return;
    }

    QByteArray idData = id.toUtf8();
    QByteArray pwData = pw.toUtf8();
    idData.resize(16, '\0');
    pwData.resize(16, '\0');

    socket->write(idData);
    socket->write(pwData);
    socket->flush();

    if (!socket->waitForReadyRead(2000))
    {
        QMessageBox::warning(this, "응답 오류", "서버 응답이 없습니다.");
        return;
    }

    QByteArray response = socket->read(1);
    qDebug()<<response;

    if (!response.isEmpty() && response[0] == '1')
    {
        stacked->setCurrentIndex(1);
    }
    else
    {
        QMessageBox::warning(this, "로그인 실패", "ID 또는 비밀번호가 틀렸습니다.");
        if (socket->state() == QAbstractSocket::ConnectedState)
            socket->disconnectFromHost();
    }

    ui->line_edit_id->clear();
    ui->line_edit_pw->clear();
}

void Page1st::draw_parking_labels()
{
    QPixmap pixmap(":/images/parking.png");
    QPainter painter(&pixmap);

    QFont font("Arial", 20, QFont::Bold);
    painter.setFont(font);
    painter.setPen(Qt::white);

    struct Label
    {
        QString text;
        QPoint pos;
        qreal angle;
    };

    QList<Label> labels = {
                           { "Dynamic Parking", QPoint(50, 140), -30 },
                           { "Guidance System", QPoint(350, 40), 30 },
                           };


    for (const Label& label : labels)
    {
        painter.save();
        painter.translate(label.pos);
        painter.rotate(label.angle);

        QPoint shadowOffset(4, 4);
        QColor shadowColor(0, 0, 0, 60);
        painter.setPen(shadowColor);
        painter.drawText(shadowOffset, label.text);

        painter.setPen(Qt::white);
        painter.drawText(QPoint(0, 0), label.text);

        painter.restore();
    }

    ui->label_parking->setPixmap(pixmap);
}
