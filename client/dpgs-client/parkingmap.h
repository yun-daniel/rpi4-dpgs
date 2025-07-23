#ifndef PARKINGMAP_H
#define PARKINGMAP_H

#include <QWidget>

namespace Ui {
class parkingMap;
}

class parkingMap : public QWidget
{
    Q_OBJECT

public:
    explicit parkingMap(QWidget *parent = nullptr);
    ~parkingMap();

private:
    Ui::parkingMap *ui;
};

#endif // PARKINGMAP_H
