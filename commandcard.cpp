#include "commandcard.h"
#include "commandmanager.h"

CommandCard::CommandCard(QObject *parent)
    : QObject(parent), cooldownMs(0)
{
}

bool CommandCard::canUse() const
{
    qint64 now = CommandManager::instance()->currentGameMs();
    return (now - m_lastUsedMs) >= qint64(cooldownMs);
}

void CommandCard::used()
{
    m_lastUsedMs = CommandManager::instance()->currentGameMs();
}

bool CommandCard::canUpgrade() const
{
    return level < maxLevel;
}

void CommandCard::doUpgrade()
{
    if (!canUpgrade()) return;
    ++level;
    // simple effect: reduce cooldown by 500ms per level
    cooldownMs = qMax(1000, cooldownMs - 500);
    // increase upgrade cost for next level
    upgradeCost = upgradeCost * 2;
}
