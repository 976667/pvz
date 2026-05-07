#include "sunflower.h"
#include <QDebug>
#include "weather.h"

SunFlower::SunFlower()
{
    hp = 300;
    time = int(10.0 * 1000 / 33);
    setMovie(":/images/SunFlower.gif");
}

void SunFlower::advance(int phase)
{
    if (!phase)
        return;
    update();
    if (hp <= 0)
        delete this;
    else
    {
        qreal factor = 1.0;
        switch (WeatherManager::instance()->current())
        {
        case Weather::Rain:
            factor = 0.8;
            break;
        case Weather::Snow:
            factor = 1.1;
            break;
        case Weather::Wind:
            factor = 1.0;
            break;
        case Weather::Sandstorm:
            factor = 1.4;
            break;
        case Weather::Clear:
        default:
            factor = 1.0;
            break;
        }

        int baseTime = time;
        if (level >= 2) baseTime = int(time * 0.8);
        if (level >= 3) baseTime = int(time * 0.6);
        
        int currentTime = qMax(1, int(baseTime * factor));
        if (++counter >= currentTime)
        {
            counter = 0;
            scene()->addItem(new Sun(pos()));
            gainExperience(4);
        }
    }
}
