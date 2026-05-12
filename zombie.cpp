#include "zombie.h"
#include "plant.h"
#include "commandmanager.h"
#include <QDebug>

Zombie::Zombie()
{
    movie = head = nullptr;
    hp = atk = state = 0;
    speed = 0.0;
}

Zombie::~Zombie()
{
    delete movie;
    delete head;
}

QRectF Zombie::boundingRect() const
{
    return QRectF(-80, -100, 200, 140);
}

void Zombie::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (!movie)
        return;

    QImage image = movie->currentImage();

    // 减速效果：速度<0.55时将图像染蓝
    if (speed < 0.55 && state != 3)
    {
        if (state != 2)
            movie->setSpeed(50);

        int w = image.width();
        int h = image.height();
        for (int i = 0; i < h; ++i)
        {
            uchar *line = image.scanLine(i);
            for (int j = 5; j < w - 5; ++j)
                line[j << 2] = 200;
        }
    }

    painter->drawImage(QRectF(-70, -100, 140, 140), image);

    // 绘制头部动画
    if (head)
    {
        image = head->currentImage();
        if (speed < 0.55)
        {
            int w = image.width();
            int h = image.height();
            for (int i = 0; i < h; ++i)
            {
                uchar *line = image.scanLine(i);
                for (int j = 5; j < w - 5; ++j)
                    line[j << 2] = 200;
            }
        }
        painter->drawImage(QRectF(0, -100, 140, 140), image);
    }
}

bool Zombie::collidesWithItem(const QGraphicsItem *other, Qt::ItemSelectionMode mode) const
{
    Q_UNUSED(mode)
    return other->type() == Plant::Type && qFuzzyCompare(other->y(), y()) && qAbs(other->x() - x()) < 30;
}

int Zombie::type() const
{
    return Type;
}

void Zombie::setMovie(QString path)
{
    if (movie)
        delete movie;
    movie = new QMovie(path);
    movie->start();
}

void Zombie::setHead(QString path)
{
    if (head)
        delete head;
    head = new QMovie(path);
    head->start();
}

int Zombie::getEffectiveHp() const
{
    CommandManager *mgr = CommandManager::instance();
    double playerMult = mgr->speedMultiplier();
    double plantBalanceMult = getPlantBalanceMultiplier();
    double balanceMult = 1.0 + (playerMult - 1.0) * 0.5 + (plantBalanceMult - 1.0) * 0.5;
    return int(double(hp) * balanceMult);
}

int Zombie::getEffectiveAtk() const
{
    CommandManager *mgr = CommandManager::instance();
    double playerMult = mgr->speedMultiplier();
    double plantBalanceMult = getPlantBalanceMultiplier();
    double balanceMult = 1.0 + (playerMult - 1.0) * 0.3 + (plantBalanceMult - 1.0) * 0.4;
    return int(double(atk) * balanceMult);
}

double Zombie::getPlantBalanceMultiplier() const
{
    double avgLevel = CommandManager::instance()->getAveragePlantLevel();
    return 1.0 + (avgLevel - 1.0) * 0.5;
}
