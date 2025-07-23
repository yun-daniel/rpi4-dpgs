#ifndef PAGE1ST_H
#define PAGE1ST_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Page1st; }
QT_END_NAMESPACE

class QStackedWidget;
class QTcpSocket;

class Page1st : public QWidget
{
    Q_OBJECT

public:
    explicit Page1st(QStackedWidget* parent_stacked, QTcpSocket *sharedSocket, QWidget *parent = nullptr);
    ~Page1st();

private slots:
    void handle_login_button_clicked();

private:
    void setup_connections();
    void draw_parking_labels();

private:
    Ui::Page1st *ui;
    QStackedWidget* stacked;
    QTcpSocket *socket;
};

#endif
