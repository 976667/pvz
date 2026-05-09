#include "pea.h"
#include "zombie.h"
#include "weather.h"
#include "plant.h"
#include "commandmanager.h"
#include <QRandomGenerator>

Pea::Pea(int attack, bool flag, Plant *parentPlant)
{
    snow = flag;
    atk = attack;
    speed = 360.0 * 33 / 1000;
    m_parentPlant = parentPlant;

    if (m_parentPlant) {
        m_attackMultiplier = m_parentPlant->getAttackMultiplier();
    } else {
        m_attackMultiplier = 1.0;
    }
}

QRectF Pea::boundingRect() const
{
    return QRectF(-12, -28, 24, 24);
}

void Pea::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    painter->drawPixmap(QRect(-12, -28, 24, 24), QPixmap(snow ? ":/images/PeaSnow.png" : ":/images/Pea.png"));
}

bool Pea::collidesWithItem(const QGraphicsItem *other, Qt::ItemSelectionMode mode) const
{
    Q_UNUSED(mode)
    return other->type() == Zombie::Type && qFuzzyCompare(other->y(), y()) && qAbs(other->x() - x()) < 15;
}

void Pea::advance(int phase)
{
    if (!phase)
        return;
    update();

    qreal factor = 1.0;
    switch (WeatherManager::instance()->current())
    {
    case Weather::Wind:
        factor = 1.3;
        break;
    case Weather::Rain:
        factor = 1.0;
        break;
    case Weather::Snow:
        factor = 0.75;
        break;
    case Weather::Sandstorm:
        factor = 0.65;
        break;
    case Weather::Clear:
    default:
        factor = 1.0;
        break;
    }

    QList<QGraphicsItem *> items = collidingItems();
    if (!items.isEmpty())
    {
        Zombie *zombie = qgraphicsitem_cast<Zombie *>(items[QRandomGenerator::global()->bounded(items.size())]);
        int effectiveAtk = int(atk * m_attackMultiplier);
        zombie->hp -= effectiveAtk;
        if (m_parentPlant) {
            m_parentPlant->gainExperience(2);
        }
        if (snow && zombie->speed > 0.55)
            zombie->speed /= 2;
        delete this;
        return;
    }
    setX(x() + speed * factor * CommandManager::instance()->speedMultiplier());
    if (x() > 1069)
        delete this;
}
