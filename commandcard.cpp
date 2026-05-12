#include "commandcard.h"
#include "commandmanager.h"
CommandCard::CommandCard(QObject *parent)
    : QObject(parent), cooldownMs(0)
{
    for (int i = 0; i < 4; ++i) {
        upgradeEffects[i] = UpgradeEffect();
    }
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
    return level < maxLevel && CommandManager::instance()->cardById(id);
}
bool CommandCard::canGainLevel() const
{
    return level < maxLevel;
}
int CommandCard::getExpForNextLevel() const
{
    static const int expTable[] = {0, 100, 300, 600, 1000};
    if (level >= maxLevel) return INT_MAX;
    return expTable[level];
}
CommandCard::UpgradeEffect CommandCard::getEffectForLevel(int lvl) const
{
    if (lvl <= 0) lvl = 1;
    if (lvl > maxLevel) lvl = maxLevel;
    return upgradeEffects[lvl - 1];
}
void CommandCard::addExperience(int exp)
{
    if (!canGainLevel()) return;
    experience += exp;
    while (experience >= getExpForNextLevel() && canGainLevel()) {
        experience -= getExpForNextLevel();
        ++level;
        applyUpgradeEffect();
    }
}
void CommandCard::doUpgrade()
{
    if (!canUpgrade()) return;
    ++level;
    if (selectedPath == PATH_SPEED) {
        cooldownMs = qMax(1000, int(cooldownMs * 0.85));
    } else if (selectedPath == PATH_ECONOMY) {
        if (params.contains("amount")) {
            params["amount"] = params["amount"].toInt() + 25;
        }
    } else if (selectedPath == PATH_POWER) {
        if (params.contains("multiplier")) {
            params["multiplier"] = params["multiplier"].toDouble() + 0.5;
        }
    }
    upgradeCost = upgradeCost * 2;
}
void CommandCard::applyUpgradeEffect()
{
    if (selectedPath == PATH_SPEED) {
        UpgradeEffect &eff = upgradeEffects[level - 1];
        eff.cooldownMultiplier = 1.0 - (level * 0.15);
        if (eff.cooldownMultiplier < 0.5) eff.cooldownMultiplier = 0.5;
    } else if (selectedPath == PATH_ECONOMY) {
        UpgradeEffect &eff = upgradeEffects[level - 1];
        eff.sunBonus = level * 10;
        eff.effectValueMultiplier = 1.0 + (level * 0.2);
    } else if (selectedPath == PATH_POWER) {
        UpgradeEffect &eff = upgradeEffects[level - 1];
        eff.effectValueMultiplier = 1.0 + (level * 0.3);
    }
}
