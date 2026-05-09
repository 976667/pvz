#ifndef SUNFLOWER_H
#define SUNFLOWER_H

#include "plant.h"
#include "sun.h"

class SunFlower : public Plant
{
public:
    SunFlower();
    void advance(int phase) override;
    double getCooldownMultiplier() const override;
};

#endif // SUNFLOWER_H
