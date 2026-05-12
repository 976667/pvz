#ifndef WALLNUT_H
#define WALLNUT_H
#include "plant.h"
class WallNut : public Plant
{
public:
    WallNut();
    void advance(int phase) override;
    int getMaxHp() const override { return maxHp; }
    double getHpMultiplier() const override;
    void checkLevelUp() override;
private:
    int counter;
    int time;
    int maxHp;
    int baseHp;
    int lastHp;
};
#endif // WALLNUT_H
