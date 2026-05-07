#include "snowpea.h"
#include "commandmanager.h"

SnowPea::SnowPea()
{
    atk = 25;
    hp = 300;
    time = int(1.4 * 1000 / 33);
    setMovie(":/images/SnowPea.gif");
}

void SnowPea::advance(int phase)
{
    if (!phase)
        return;
    update();
    if (hp <= 0)
        delete this;
    else if (++counter >= CommandManager::instance()->effectiveTicksFor(time))
    {
        counter = 0;
        if (!collidingItems().isEmpty())
        {
            int effectiveAtk = int(atk * getAttackMultiplier());
            Pea *pea = new Pea(effectiveAtk, true, this);
            pea->setX(x() + 32);
            pea->setY(y());
            scene()->addItem(pea);
            return;
        }
    }
}

bool SnowPea::collidesWithItem(const QGraphicsItem *other, Qt::ItemSelectionMode mode) const
{
    Q_UNUSED(mode)
    return other->type() == Zombie::Type && qFuzzyCompare(other->y(), y());
}
