#include "page1st.h"
#include "ui_page1st.h"

#include <QDebug>
#include <QMessageBox>
#include <QStackedWidget>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QFile>
#include <QPainter>
#include <QRegularExpression>
#include <QGraphicsDropShadowEffect>


Page1st::Page1st(QStackedWidget *parent_stacked, QSslSocket *sharedSocket, QWidget *parent)
    : QWidget{parent}
    , ui(new Ui::Page1st)
    , stacked(parent_stacked)
    , socket(sharedSocket)
{
    ui->setupUi(this);
    setup_connections();
    draw_parking_labels();

    ui->line_edit_id->setPlaceholderText("ID");
    ui->line_edit_id->addAction(QIcon(":/images/user.png"), QLineEdit::LeadingPosition);

    ui->line_edit_pw->setPlaceholderText("Password");
    ui->line_edit_pw->setEchoMode(QLineEdit::Password);
    ui->line_edit_pw->addAction(QIcon(":/images/password.png"), QLineEdit::LeadingPosition);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(50);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 150));

    ui->groupBox->setGraphicsEffect(shadow);

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

    ui->label_result1->clear();
    ui->label_result2->clear();

    auto clearInputs = [&]() {
        ui->line_edit_id->clear();
        ui->line_edit_pw->clear();
    };

    if (id.isEmpty() || pw.isEmpty()) {
        ui->label_result1->setText("Login failed");
        ui->label_result2->setText("Please enter both ID and password.");
        clearInputs();
        return;
    }

    static const QRegularExpression passwordPattern(
        "^(?=.*[a-zA-Z])(?=.*\\d)(?=.*[!@#$%^&*]).{8,}$"
        );

    if (!passwordPattern.match(pw).hasMatch()) {
        ui->label_result1->setText("Login failed");
        ui->label_result2->setText("Password must contain:\n"
                                   "- At least one letter\n"
                                   "- At least one number\n"
                                   "- At least one special character (!@#$%^&*)\n"
                                   "- At least 8 characters total");
        clearInputs();
        return;
    }

    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->abort();
    }

    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    QString certPath = QCoreApplication::applicationDirPath() + "/../../certs/Final.crt";
    QFile certFile(certPath);
    if (certFile.open(QIODevice::ReadOnly)) {
        QSslCertificate caCert(&certFile, QSsl::Pem);
        sslConfig.setCaCertificates({ caCert });
        sslConfig.setPeerVerifyMode(QSslSocket::VerifyPeer);
        socket->setSslConfiguration(sslConfig);
    } else {
        ui->label_result1->setText("Login failed");
        ui->label_result2->setText("Failed to open certificate file.");
        clearInputs();
        return;
    }

    connect(socket, &QSslSocket::sslErrors, this, [](const QList<QSslError> &errors) {
        qDebug() << "[TLS] SSL 오류 발생:";
        for (const QSslError &error : errors) {
            qDebug() << " - " << error.errorString();
        }
    });

    socket->connectToHostEncrypted("192.168.0.32", 9999);

    if (!socket->waitForEncrypted(2000)) {
        ui->label_result1->setText("Login failed");
        ui->label_result2->setText("Failed to establish TLS connection.");
        clearInputs();
        return;
    }

    QByteArray idData = id.toUtf8();
    QByteArray pwData = pw.toUtf8();
    idData.resize(16, '\0');
    pwData.resize(16, '\0');

    socket->write(idData);
    socket->write(pwData);
    socket->flush();

    if (!socket->waitForReadyRead(3000)) {
        ui->label_result1->setText("Login failed");
        ui->label_result2->setText("No response from server.");
        clearInputs();
        return;
    }

    QByteArray response = socket->read(1);

    if (!response.isEmpty() && response[0] == '1') {
        ui->label_result1->clear();
        ui->label_result2->clear();
        stacked->setCurrentIndex(1);
    } else {
        ui->label_result1->setText("Login failed");
        ui->label_result2->setText("ID or password is incorrect.");
        if (socket->state() == QAbstractSocket::ConnectedState)
            socket->disconnectFromHost();
        clearInputs();
    }
}

void Page1st::draw_parking_labels()
{
    QPixmap pixmap(":/images/parking.png");

    QPainter painter(&pixmap);

    QFont font("Arial", 30, QFont::Bold);
    painter.setFont(font);
    painter.setPen(Qt::white);

    struct Label {
        QString text;
        QPoint pos;
        qreal angle;
    };

    QList<Label> labels = {
                           { "Dynamic Parking", QPoint(10, 180), -30 },
                           { "Guidance System", QPoint(360, 25), 30 },
                           };

    for (const Label& label : labels) {
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
