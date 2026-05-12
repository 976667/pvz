#ifndef COMMANDCARD_H
#define COMMANDCARD_H
#include <QObject>
#include <QVariantMap>
class CommandCard : public QObject
{
    Q_OBJECT
public:
    enum UpgradePath {
        PATH_SPEED,      // 速度路线：冷却时间减少
        PATH_POWER,      // 强化路线：效果增强
        PATH_ECONOMY     // 经济路线：产出增加
    };
    struct UpgradeEffect {
        double cooldownMultiplier = 1.0;
        double effectValueMultiplier = 1.0;
        double sunBonus = 0;
    };
    explicit CommandCard(QObject *parent = nullptr);
    QString id;
    QString name;
    QString iconPath;
    int cooldownMs;
    int level = 1;
    int maxLevel = 3;
    int upgradeCost = 1000;
    int experience = 0;
    QVariantMap params;
    UpgradePath selectedPath = PATH_SPEED;
    UpgradeEffect upgradeEffects[4];
    bool canUpgrade() const;
    bool canGainLevel() const;
    void doUpgrade();
    void applyUpgradeEffect();
    void addExperience(int exp);
    UpgradeEffect getEffectForLevel(int lvl) const;
    int getExpForNextLevel() const;
    bool canUse() const;
    void used();
private:
    qint64 m_lastUsedMs = 0;
};
#endif // COMMANDCARD_H
