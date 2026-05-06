#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include <QObject>
#include <QMap>
#include "commandcard.h"
#include <QPointer>
#include <QVariantMap>

class QTimer;

class CommandManager : public QObject
{
    Q_OBJECT
public:
    static CommandManager* instance();
    ~CommandManager() override;

    void syncToGameTimer(QTimer *gameTimer, int baseIntervalMs = 33);
    qint64 currentGameMs() const { return m_gameMs; }
    int cooldownMsFor(const QString &id) const;
    CommandCard* cardById(const QString &id) const;

    void addDefaultCards();
    // attempt to use card by id, returns effect value (e.g., sun amount) or 0 if cannot use
    int useCard(const QString &id);

signals:
    void cardUsed(const QString &id, const QVariantMap &params);

private:
    explicit CommandManager(QObject *parent = nullptr);
    static CommandManager *s_instance;
    QMap<QString, CommandCard*> m_cards;
    QPointer<QTimer> m_gameTimer;
    double m_baseIntervalMs = 33.0;
    qint64 m_gameMs = 0;
    double m_userSpeedMultiplier = 1.0;
    double m_effectSpeedMultiplier = 1.0;

public:
    void setUserSpeedMultiplier(double m) { m_userSpeedMultiplier = m; }
    void setEffectSpeedMultiplier(double m) { m_effectSpeedMultiplier = m; }
    double userSpeedMultiplier() const { return m_userSpeedMultiplier; }
    double effectSpeedMultiplier() const { return m_effectSpeedMultiplier; }
    double speedMultiplier() const { return m_userSpeedMultiplier * m_effectSpeedMultiplier; }

    int effectiveTicksFor(int baseTicks) const;

private slots:
    void onGameTick();
};

#endif // COMMANDMANAGER_H
