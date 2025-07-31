#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "page1st.h"
#include "page2nd.h"

#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QPixmap>
#include <QSslSocket>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , sharedSocket(new QSslSocket(this))
    , page1(nullptr)
    , page2(nullptr)
{
    ui->setupUi(this);
    setFixedSize(1400, 800);
    setWindowTitle("Dynamic Parking Guidance System Project");

    ui->stackedWidget->setCurrentIndex(0);

    page1 = new Page1st(ui->stackedWidget, sharedSocket, ui->page_1);
    page2 = new Page2nd(ui->stackedWidget, sharedSocket, ui->page_2);
}

MainWindow::~MainWindow()
{
    delete ui;
}
