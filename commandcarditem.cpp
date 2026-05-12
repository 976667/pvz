#include "commandcarditem.h"
#include "commandmanager.h"
#include "shop.h"
#include <QPainter>
#include <QRandomGenerator>
#include "commandmanager.h"

CommandCardItem::CommandCardItem(const QString &cardId, const QString &t, int cooldownMs, QGraphicsItem *parent)
    : Other(), id(cardId), counter(1000000), text(t)
{
    setParentItem(parent);
    // convert ms -> ticks (~33ms per tick)
    cooldownTicks = qMax(1, int((cooldownMs + 16) / 33));
    cooldownMsVal = cooldownMs;
    usedAtMs = 0;
}

QRectF CommandCardItem::boundingRect() const
{
    // horizontal compact card: width 120, height 40
    return QRectF(-60, -20, 120, 40);
}

void CommandCardItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    QRectF br = boundingRect();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(245, 245, 245, 230));
    painter->drawRoundedRect(br, 6, 6);

    QRectF iconRect(br.left() + 6, br.top() + 8, 24, 24);
    QPixmap icon = QPixmap(":/images/Sun.png").scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    painter->drawPixmap(iconRect.toRect(), icon);

    QRectF textRect(iconRect.right() + 8, br.top() + 4, br.width() - iconRect.width() - 22, br.height() - 8);
    QFont font;
    font.setPointSizeF(11);
    painter->setFont(font);
    painter->setPen(Qt::black);
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);

    CommandCard *card = CommandManager::instance()->cardById(id);
    if (card) {
        QString lvl = QString("Lv%1").arg(card->level);
        QFont f2;
        f2.setPointSizeF(9);
        painter->setFont(f2);
        QRectF badgeRect(textRect.right() - 36, textRect.top(), 34, 16);
        painter->setBrush(QColor(200,200,200,220));
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(badgeRect, 4, 4);
        painter->setPen(Qt::black);
        painter->drawText(badgeRect, Qt::AlignCenter, lvl);

        if (card->level < card->maxLevel) {
            int expNeeded = card->getExpForNextLevel();
            if (expNeeded > 0) {
                QRectF expBarRect(textRect.left(), textRect.bottom() - 6, textRect.width(), 4);
                painter->setBrush(QColor(180, 180, 180, 200));
                painter->setPen(Qt::NoPen);
                painter->drawRect(expBarRect);

                qreal expProgress = qreal(card->experience) / qreal(expNeeded);
                QRectF expFillRect(expBarRect.left(), expBarRect.top(),
                                   expBarRect.width() * expProgress, expBarRect.height());
                if (card->selectedPath == CommandCard::PATH_SPEED)
                    painter->setBrush(QColor(100, 180, 255, 200));
                else if (card->selectedPath == CommandCard::PATH_ECONOMY)
                    painter->setBrush(QColor(255, 215, 0, 200));
                else
                    painter->setBrush(QColor(255, 100, 100, 200));
                painter->drawRect(expFillRect);
            }
        }

        if (upgradeFlashMs > 0) {
            QColor flashColor(255, 255, 200, 150);
            painter->setBrush(flashColor);
            painter->drawRoundedRect(br, 6, 6);
        }

        if (showUpgradePreview && card->canUpgrade()) {
            QRectF previewRect(br.left(), br.bottom() + 2, br.width(), 30);
            painter->setBrush(QColor(50, 50, 50, 240));
            painter->setPen(Qt::white);
            painter->drawRoundedRect(previewRect, 4, 4);

            QString previewText;
            if (card->selectedPath == CommandCard::PATH_SPEED) {
                int newCooldown = int(card->cooldownMs * 0.85);
                previewText = QString("冷却: %1ms → %2ms").arg(card->cooldownMs).arg(newCooldown);
            } else if (card->selectedPath == CommandCard::PATH_ECONOMY) {
                int currentAmount = card->params.value("amount", 0).toInt();
                int newAmount = currentAmount + 25;
                previewText = QString("产出: %1 → %2 (+25)").arg(currentAmount).arg(newAmount);
            } else if (card->selectedPath == CommandCard::PATH_POWER) {
                double currentMult = card->params.value("multiplier", 1.0).toDouble();
                double newMult = currentMult + 0.5;
                previewText = QString("效果: %1x → %2x").arg(currentMult, 0, 'f', 1).arg(newMult, 0, 'f', 1);
            }

            QFont previewFont;
            previewFont.setPointSizeF(8);
            painter->setFont(previewFont);
            painter->drawText(previewRect, Qt::AlignCenter, previewText);
        }
    }

    if (usedAtMs != 0) {
        qint64 elapsed = CommandManager::instance()->currentGameMs() - usedAtMs;
        if (elapsed < cooldownMsVal) {
            qreal progress = qreal(elapsed) / qreal(cooldownMsVal);
            qreal overlayWidth = textRect.width() * (1.0 - progress);
            QRectF overlayRect(textRect.right() - overlayWidth, br.top(), overlayWidth, br.height());
            painter->setBrush(QColor(0, 0, 0, 140));
            painter->setPen(Qt::NoPen);
            painter->drawRect(overlayRect);
        }
    }
}

void CommandCardItem::advance(int phase)
{
    if (!phase) return;
    update();
    if (counter < cooldownTicks) ++counter;
    if (upgradeFlashMs > 0) upgradeFlashMs -= 33;
}

void CommandCardItem::setReady()
{
    // 设置卡片为就绪状态，让usedAtMs为0，表示冷却已结束
    usedAtMs = 0;
    counter = cooldownTicks;  // 让计数器直接达到最大值，表示可以立即使用
}

void CommandCardItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    int added = CommandManager::instance()->useCard(id);
    if (added > 0)
    {
        // find Shop instance in the scene (use known shop coords as primary lookup)
        Shop *s = nullptr;
        if (scene()) {
            QList<QGraphicsItem *> shopItems = scene()->items(QPointF(300, 15));
            if (!shopItems.isEmpty())
                s = qgraphicsitem_cast<Shop *>(shopItems[0]);
            if (!s) {
                // fallback: search entire scene for Shop
                const QList<QGraphicsItem *> all = scene()->items();
                foreach (QGraphicsItem *it, all) {
                    s = qgraphicsitem_cast<Shop *>(it);
                    if (s) break;
                }
            }
        }
        if (id == "instant_sun" && s) s->sun += added;
        // record cooldown start for all successfully used cards so overlay appears
        counter = 0;
        usedAtMs = CommandManager::instance()->currentGameMs();
    }
}

void CommandCardItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    // attempt upgrade: find Shop and card
    Shop *s = nullptr;
    if (scene()) {
        QList<QGraphicsItem *> shopItems = scene()->items(QPointF(300, 15));
        if (!shopItems.isEmpty())
            s = qgraphicsitem_cast<Shop *>(shopItems[0]);
        if (!s) {
            const QList<QGraphicsItem *> all = scene()->items();
            foreach (QGraphicsItem *it, all) {
                s = qgraphicsitem_cast<Shop *>(it);
                if (s) break;
            }
        }
    }
    CommandCard *card = CommandManager::instance()->cardById(id);
    if (!card || !s) return;
    if (!card->canUpgrade()) return;
    int cost = card->upgradeCost;
    if (s->sun < cost) return;
    s->sun -= cost;
    card->doUpgrade();
    upgradeFlashMs = 1000; // show for ~1s via paint/advance
    update();
}
