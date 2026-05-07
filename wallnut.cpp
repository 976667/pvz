#include "wallnut.h"
#include <QDebug>

WallNut::WallNut()
{
    maxHp = 4000;
    hp = maxHp;
    lastHp = maxHp;
    state = 0;
    counter = 0;
    time = 200;
    setMovie(":/images/WallNut.gif");
}

void WallNut::advance(int phase)
{
    if (!phase)
        return;
    update();
    
    static int lastLevel = 1;
    if (level != lastLevel) {
        if (level == 2) {
            maxHp = 6000;
        } else if (level == 3) {
            maxHp = 9000;
        }
        hp = maxHp;
        lastHp = maxHp;
        lastLevel = level;
    }
    
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
    
    if (hp <= maxHp / 3 && state != 2)
    {
        state = 2;
        setMovie(":/images/WallNut2.gif");
    }
    else if (maxHp / 3 < hp && hp <= maxHp * 2 / 3 && state != 1)
    {
        state = 1;
        setMovie(":/images/WallNut1.gif");
    }
    
    if (++counter >= time)
    {
        counter = 0;
        gainExperience(5);
    }
}
