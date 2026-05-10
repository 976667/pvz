#include "weather.h"
#include "commandmanager.h"
#include <QTimer>
#include <QRandomGenerator>

WeatherManager* WeatherManager::s_instance = nullptr;

WeatherManager::WeatherManager(QObject *parent)
    : QObject(parent), m_current(Weather::Clear)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &WeatherManager::nextWeather);
}

WeatherManager* WeatherManager::instance()
{
    if (!s_instance)
        s_instance = new WeatherManager;
    return s_instance;
}

void WeatherManager::start(int intervalMs)
{
    m_intervalMs = double(intervalMs);
    m_accumulatedMs = 0.0;
    if (m_gameTimer)
        return;
    if (m_timer->isActive())
        m_timer->stop();
    if (!m_gameTimer)
        m_timer->start(intervalMs);
}

void WeatherManager::syncToGameTimer(QTimer *gameTimer, int baseIntervalMs)
{
    if (m_gameTimer) {
        disconnect(m_gameTimer.data(), &QTimer::timeout, this, &WeatherManager::onGameTick);
    }
    m_gameTimer = gameTimer;
    m_baseIntervalMs = double(baseIntervalMs);
    m_accumulatedMs = 0.0;
    if (m_gameTimer) {
        connect(m_gameTimer.data(), &QTimer::timeout, this, &WeatherManager::onGameTick);
    }
}

void WeatherManager::onGameTick()
{
    if (!m_gameTimer) return;
    if (m_baseIntervalMs <= 0.0) return;

    double speedMult = CommandManager::instance()->speedMultiplier();
    m_accumulatedMs += m_baseIntervalMs * speedMult;
    if (m_accumulatedMs >= m_intervalMs) {
        m_accumulatedMs -= m_intervalMs;
        nextWeather();
    }
}

void WeatherManager::nextWeather()
{
    static const Weather options[] = {
        Weather::Clear,
        Weather::Rain,
        Weather::Snow,
        Weather::Wind,
        Weather::Sandstorm
    };

    Weather w = m_current;
    const int optionCount = int(sizeof(options) / sizeof(options[0]));
    while (w == m_current)
        w = options[QRandomGenerator::global()->bounded(optionCount)];

    if (w != m_current) {
        m_current = w;
        emit weatherChanged(w);
    }
}
