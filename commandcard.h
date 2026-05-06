#ifndef COMMANDCARD_H
#define COMMANDCARD_H

#include <QObject>
#include <QVariantMap>

class CommandCard : public QObject
{
    Q_OBJECT
public:
    explicit CommandCard(QObject *parent = nullptr);
    QString id;
    QString name;
    QString iconPath;
    int cooldownMs;
    int level = 1;
    int maxLevel = 3;
    int upgradeCost = 200; // default cost
    QVariantMap params; // effect parameters

    bool canUpgrade() const;
    void doUpgrade();

    bool canUse() const;
    void used();

private:
    qint64 m_lastUsedMs = 0;
};

#endif // COMMANDCARD_H
