#ifndef PAGE1ST_H
#define PAGE1ST_H

#include <QWidget>
#include <QSslSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class Page1st; }
QT_END_NAMESPACE

class QStackedWidget;

class Page1st : public QWidget
{
    Q_OBJECT

public:
    explicit Page1st(QStackedWidget *parent_stacked, QSslSocket *sharedSocket, QWidget *parent = nullptr);
    ~Page1st();

private slots:
    void handle_login_button_clicked();

private:
    void setup_connections();
    void draw_parking_labels();

private:
    Ui::Page1st *ui;
    QStackedWidget *stacked;
    QSslSocket *socket;
};

#endif
