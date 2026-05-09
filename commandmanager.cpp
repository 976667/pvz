#include "commandmanager.h"
#include <QDebug>
#include <QTimer>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include "plant.h"

// for QPointer
#include <QPointer>

CommandManager* CommandManager::s_instance = nullptr;

CommandManager::CommandManager(QObject *parent)
    : QObject(parent)
{
    m_gameMs = 0;
    m_baseIntervalMs = 33.0;
}

CommandManager::~CommandManager()
{
    qDeleteAll(m_cards);
}

CommandManager* CommandManager::instance()
{
    if (!s_instance)
        s_instance = new CommandManager;
    return s_instance;
}

int CommandManager::cooldownMsFor(const QString &id) const
{
    if (!m_cards.contains(id)) return 0;
    return m_cards[id]->cooldownMs;
}

CommandCard* CommandManager::cardById(const QString &id) const
{
    if (!m_cards.contains(id)) return nullptr;
    return m_cards.value(id);
}

void CommandManager::addDefaultCards()
{
    // add multiple default command cards if not present
    if (!m_cards.contains("instant_sun")) {
        CommandCard *c = new CommandCard(this);
        c->id = "instant_sun";
        c->name = "Sun Card";
        c->cooldownMs = 5000; // 5s cooldown
        c->params["amount"] = 150;
        m_cards.insert(c->id, c);
    }
    if (!m_cards.contains("speed_boost")) {
        CommandCard *c = new CommandCard(this);
        c->id = "speed_boost";
        c->name = "Speed Boost";
        c->cooldownMs = 5000;
        c->params["durationMs"] = 5000;
        c->params["multiplier"] = 2; // 2x speed
        m_cards.insert(c->id, c);
    }
    
}

int CommandManager::useCard(const QString &id)
{
    if (!m_cards.contains(id))
        return 0;
    CommandCard *c = m_cards[id];
    if (!c->canUse())
        return 0;
    c->used();
    emit cardUsed(c->id, c->params);
    // for instant_sun return amount
    if (id == "instant_sun")
        return c->params.value("amount", 0).toInt();
    // for other cards, return 1 to indicate success (no sun amount)
    return 1;
}

void CommandManager::syncToGameTimer(QTimer *gameTimer, int baseIntervalMs)
{
    if (m_gameTimer) {
        disconnect(m_gameTimer.data(), &QTimer::timeout, this, &CommandManager::onGameTick);
    }
    m_gameTimer = gameTimer;
    m_baseIntervalMs = double(baseIntervalMs);
    if (m_gameTimer) {
        connect(m_gameTimer.data(), &QTimer::timeout, this, &CommandManager::onGameTick);
    }
}

void CommandManager::onGameTick()
{
    m_gameMs += qint64(m_baseIntervalMs * speedMultiplier());
}

int CommandManager::effectiveTicksFor(int baseTicks) const
{
    double mult = qMax(1.0, speedMultiplier());
    return qMax(1, int(double(baseTicks) / mult));
}

double CommandManager::getAveragePlantLevel() const
{
    if (!m_gameTimer || !m_gameTimer->parent())
        return 1.0;

    QGraphicsScene *scene = qobject_cast<QGraphicsScene *>(m_gameTimer->parent());
    if (!scene)
        return 1.0;

    QList<QGraphicsItem *> items = scene->items();
    int totalLevel = 0;
    int plantCount = 0;

    for (QGraphicsItem *item : items) {
        Plant *plant = qgraphicsitem_cast<Plant *>(item);
        if (plant) {
            totalLevel += plant->level;
            ++plantCount;
        }
    }

    if (plantCount == 0)
        return 1.0;

    return double(totalLevel) / double(plantCount);
}
