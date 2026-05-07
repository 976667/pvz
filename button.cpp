#include "button.h"
#include "commandmanager.h"

Button::Button(QMediaPlayer *s, QTimer *t)
{
    sound = s;
    timer = t;
    speedIndex = 0;
}

QRectF Button::boundingRect() const
{
    return QRectF(-80, -20, 160, 40);
}

void Button::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    static const int speedList[] = {1, 2, 4};
    Q_UNUSED(option)
    Q_UNUSED(widget)
    painter->drawPixmap(QRect(-80, -20, 160, 40), QPixmap(":/images/Button.png"));
    painter->setPen(Qt::green);
    QFont font("Calibri", 16, QFont::Bold, true);
    painter->setFont(font);
    QString text = QString("%1 x%2").arg(timer->isActive() ? "PAUSE" : "CONTINUE").arg(speedList[speedIndex]);
    if (timer->isActive())
        painter->drawText(boundingRect(), Qt::AlignCenter, text);
    else
        painter->drawText(boundingRect(), Qt::AlignCenter, text);
}

void Button::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    static const int speedList[] = {1, 2, 4};
    static const int baseInterval = 33;
    if (event->button() == Qt::LeftButton)
    {
        if (timer->isActive())
        {
            sound->stop();
            timer->stop();
        }
        else
        {
            sound->play();
            timer->start();
        }
    }
    else if (event->button() == Qt::RightButton)
    {
        speedIndex = (speedIndex + 1) % 3;
        timer->setInterval(baseInterval / speedList[speedIndex]);
        // update CommandManager user multiplier so cooldowns and durations scale with chosen speed
        CommandManager::instance()->setUserSpeedMultiplier(speedList[speedIndex]);
        if (timer->isActive())
            timer->start();
    }
    update();
}
