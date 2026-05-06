#ifndef WEATHEREFFECT_H
#define WEATHEREFFECT_H

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QList>

class Particle : public QGraphicsItem {
public:
    enum ParticleType { Raindrop, Snowflake, WindParticle, SandstormParticle };
    
    Particle(qreal x, qreal y, ParticleType type);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void advance(int phase) override;
    
private:
    ParticleType type;
    qreal velocityY;
    qreal velocityX;
    qreal lifetime;
};

#endif // WEATHEREFFECT_H
