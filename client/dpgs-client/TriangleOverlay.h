// TriangleOverlay.h
#ifndef TRIANGLEOVERLAY_H
#define TRIANGLEOVERLAY_H

#include <QWidget>
#include <QPainter>

class TriangleOverlay : public QWidget
{
public:
    explicit TriangleOverlay(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
        setAttribute(Qt::WA_TranslucentBackground);
        resize(100, 80);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QPolygon triangle;
        triangle << QPoint(width()/2, 0)
                 << QPoint(0, height())
                 << QPoint(width(), height());

        QColor color(255, 255, 100, 120); // 노란색 + 투명도
        painter.setBrush(color);
        painter.setPen(Qt::NoPen);
        painter.drawPolygon(triangle);
    }
};

#endif // TRIANGLEOVERLAY_H
