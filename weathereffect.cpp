#include "weathereffect.h"
#include <QPainter>
#include <QRandomGenerator>

Particle::Particle(qreal x, qreal y, ParticleType t)
    : QGraphicsItem(nullptr), type(t), lifetime(0)
{
    setPos(x, y);
    
    if (type == Raindrop) {
        velocityY = QRandomGenerator::global()->bounded(4) + 8;
        velocityX = QRandomGenerator::global()->bounded(2) - 1;
    } else if (type == Snowflake) {
        velocityY = QRandomGenerator::global()->bounded(2) + 1;
        velocityX = QRandomGenerator::global()->bounded(3) - 1.5;
    } else if (type == WindParticle) {
        velocityY = QRandomGenerator::global()->bounded(1) - 0.5;
        velocityX = QRandomGenerator::global()->bounded(6) + 4;
    } else { // SandstormParticle
        velocityY = QRandomGenerator::global()->bounded(3) + 2;
        velocityX = QRandomGenerator::global()->bounded(4) - 2;
    }
}

QRectF Particle::boundingRect() const
{
    if (type == Raindrop) {
        return QRectF(-2, -8, 4, 16);
    } else if (type == Snowflake) {
        return QRectF(-3, -3, 6, 6);
    } else if (type == WindParticle) {
        return QRectF(-4, -2, 8, 4);
    } else {
        return QRectF(-2, -2, 4, 4);
    }
}

void Particle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    if (type == Raindrop) {
        painter->setPen(QPen(QColor(80, 150, 255), 3));
        painter->drawLine(0, -8, 0, 8);
    } else if (type == Snowflake) {
        painter->setBrush(QColor(255, 255, 255, 200));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(-3, -3, 6, 6);
    } else if (type == WindParticle) {
        painter->setPen(QPen(QColor(200, 200, 200, 100), 2));
        painter->drawLine(-4, 0, 4, 0);
    } else { // SandstormParticle
        painter->setBrush(QColor(200, 160, 80, 180));
        painter->setPen(Qt::NoPen);
        painter->drawRect(-2, -2, 4, 4);
    }
}

void Particle::advance(int phase)
{
    if (phase == 0) return;
    
    setPos(x() + velocityX, y() + velocityY);
    lifetime += 0.5;
    
    // 粒子超出边界或生命周期已满时删除
    if (y() > 650 || lifetime > 100) {
        scene()->removeItem(this);
        delete this;
    }
}
