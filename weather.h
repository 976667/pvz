#ifndef WEATHER_H
#define WEATHER_H

#include <QObject>
#include <QPointer>

enum class Weather { Clear, Rain, Snow, Wind, Sandstorm };

class QTimer;

class WeatherManager : public QObject {
    Q_OBJECT
public:
    static WeatherManager* instance();
    Weather current() const { return m_current; }
    void start(int intervalMs = 30000);
    // 将天气定时器与游戏定时器同步：
    // gameTimer: 游戏主循环的 QTimer
    // baseIntervalMs: 在倍速 1 时游戏定时器的基准间隔（默认 33ms）
    void syncToGameTimer(QTimer *gameTimer, int baseIntervalMs = 33);
signals:
    void weatherChanged(Weather newWeather);
private slots:
    void nextWeather();
    void onGameTick();
private:
    explicit WeatherManager(QObject *parent = nullptr);
    static WeatherManager* s_instance;
    Weather m_current;
    QTimer *m_timer = nullptr;
    QPointer<QTimer> m_gameTimer;
    double m_intervalMs = 30000.0;
    double m_baseIntervalMs = 33.0;
    double m_accumulatedMs = 0.0;
};

#endif // WEATHER_H
