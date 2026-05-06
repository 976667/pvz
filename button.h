#ifndef BUTTON_H
#define BUTTON_H

#include <QTimer>
#include <QMediaPlayer>
#include "other.h"

class Button : public Other
{
public:
    Button(QMediaPlayer *s, QTimer *t);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
private:
    QMediaPlayer *sound;
    QTimer *timer;
    int speedIndex;
};

#endif // BUTTON_H
