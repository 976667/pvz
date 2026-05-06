#ifndef COMMANDCARDITEM_H
#define COMMANDCARDITEM_H

#include "other.h"

class CommandCardItem : public Other
{
public:
    CommandCardItem(const QString &id, const QString &text, int cooldownMs, QGraphicsItem *parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void advance(int phase) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

    QString id;
    int counter;
    int cooldownTicks;
    int cooldownMsVal;
    qint64 usedAtMs;
    QString text;
    // show simple temporary message after upgrade
    int upgradeFlashMs = 0;
};

#endif // COMMANDCARDITEM_H
