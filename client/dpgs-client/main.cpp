#include "mainwindow.h"

#include <QApplication>
#include <QFont>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    int id = QFontDatabase::addApplicationFont(":/images/AritaDotumKR-Medium.ttf");
    if (id != -1) {
        QString family = QFontDatabase::applicationFontFamilies(id).at(0);

        QFont font(family, 11);
        a.setFont(font);
    } else {
        qWarning("폰트 로딩 실패");
    }

    MainWindow w;
    w.show();
    return a.exec();
}
