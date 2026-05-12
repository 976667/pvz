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
    if (hp < lastHp) {//经验获取：每损失50点生命值获得1点经验
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
    if (++counter >= CommandManager::instance()->effectiveTicksFor(time))//定时被动经验：每60帧获得4点经验
    {
        counter = 0;
        gainExperience(4);
    }
    {
        counter = 0;
        gainExperience(4);
    }
}
double WallNut::getHpMultiplier() const
{
    return 1.0 + (level - 1) * 0.5;
}
void WallNut::checkLevelUp()
{
    static const int expRequired[] = {0, 500, 1500, 10000};
    if (level < 3 && experience >= expRequired[level]) {
        experience -= expRequired[level];
        ++level;
        maxHp = int(baseHp * getHpMultiplier());
        hp = maxHp;
        lastHp = maxHp;//升级后恢复生命值到满，并重置经验
    }
}
