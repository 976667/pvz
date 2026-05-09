#include "wallnut.h"
#include "commandmanager.h"
#include <QDebug>

WallNut::WallNut()
{
    baseHp = 4000;
    maxHp = baseHp;
    hp = maxHp;
    lastHp = maxHp;
    state = 0;
    counter = 0;
    time = 60;
    setMovie(":/images/WallNut.gif");
}

void WallNut::advance(int phase)
{
    if (!phase)
        return;
    update();

    maxHp = int(baseHp * getHpMultiplier());
    if (hp > maxHp) {
        hp = maxHp;
    }
    lastHp = maxHp;

    if (hp < lastHp) {
        int damageTaken = lastHp - hp;
        if (damageTaken > 0) {
            gainExperience(damageTaken / 50);
        }
        lastHp = hp;
    }

    if (hp <= 0)
    {
        delete this;
        return;
    }

    if (hp <= maxHp * 0.3 && state != 2)
    {
        state = 2;
        setMovie(":/images/WallNut2.gif");
    }
    else if (maxHp * 0.3 < hp && hp <= maxHp * 0.6 && state != 1)
    {
        state = 1;
        setMovie(":/images/WallNut1.gif");
    }

    if (++counter >= CommandManager::instance()->effectiveTicksFor(time))
    {
        counter = 0;
        gainExperience(4);
    }
}

double WallNut::getHpMultiplier() const
{
    return 1.0 + (level - 1) * 0.5;
}
