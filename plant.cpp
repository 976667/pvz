#include "plant.h"
#include "zombie.h"
#include "wallnut.h"

Plant::Plant()
{
    movie = nullptr;
    atk = counter = state = time = 0;
    experience = 0;
    level = 1;
}

Plant::~Plant()
{
    delete movie;
}

QRectF Plant::boundingRect() const
{
    return QRectF(-35, -35, 70, 70);
}

void Plant::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    if (movie)
        painter->drawImage(boundingRect(), movie->currentImage());

    painter->setRenderHint(QPainter::Antialiasing, true);
    QFont font;
    font.setPointSizeF(14);
    font->setBold(true);
    painter->setFont(font);

    QRectF badgeRect(-42, -32, 22, 22);
    painter->setBrush(QColor(255, 215, 0, 255));
    painter->setPen(QPen(Qt::black, 2));
    painter->drawEllipse(badgeRect);

    painter->setPen(Qt::black);
    QString lvl = QString("%1").arg(level);
    painter->drawText(badgeRect, Qt::AlignCenter, lvl);

    int maxHealth = getMaxHp();

    QRectF healthBarRect(-30, 38, 60, 6);
    painter->setBrush(QColor(100, 100, 100, 200));
    painter->setPen(Qt::NoPen);
    painter->drawRect(healthBarRect);

    qreal healthPercent = qreal(hp) / qreal(maxHealth);
    healthPercent = qBound(0.0, healthPercent, 1.0);
    QRectF healthFillRect(healthBarRect.left(), healthBarRect.top(),
                          healthBarRect.width() * healthPercent, healthBarRect.height());

    if (healthPercent > 0.6)
        painter->setBrush(QColor(0, 200, 0, 220));
    else if (healthPercent > 0.3)
        painter->setBrush(QColor(255, 200, 0, 220));
    else
        painter->setBrush(QColor(255, 0, 0, 220));

    painter->drawRect(healthFillRect);

    painter->setPen(QPen(Qt::black, 1));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(healthBarRect);
}

bool Plant::collidesWithItem(const QGraphicsItem *other, Qt::ItemSelectionMode mode) const
{
    Q_UNUSED(mode)
    return other->type() == Zombie::Type && qFuzzyCompare(other->y(), y()) && qAbs(other->x() - x()) < 30;
}

int Plant::type() const
{
    return Type;
}

void Plant::setMovie(QString path)
{
    if (movie)
        delete movie;
    movie = new QMovie(path);
    movie->start();
}

void Plant::gainExperience(int exp)
{
    experience += exp;
    checkLevelUp();
}

void Plant::checkLevelUp()
{
    static const int expRequired[] = {0, 500, 1500, 10000};
    if (level < 3 && experience >= expRequired[level]) {
        experience -= expRequired[level];
        ++level;
        hp = getMaxHp();
    }
}

double Plant::getAttackMultiplier() const
{
    return 1.0 + (level - 1) * 0.5;
}

double Plant::getSpeedMultiplier() const
{
    return 1.0;
}
