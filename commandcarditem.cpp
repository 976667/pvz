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
    // background
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(245, 245, 245, 230));
    painter->drawRoundedRect(br, 6, 6);

    // icon on the left
    QRectF iconRect(br.left() + 6, br.top() + 8, 24, 24);
    QPixmap icon = QPixmap(":/images/Sun.png").scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    painter->drawPixmap(iconRect.toRect(), icon);

    // text area
    QRectF textRect(iconRect.right() + 8, br.top() + 4, br.width() - iconRect.width() - 22, br.height() - 8);
    QFont font;
    font.setPointSizeF(11);
    painter->setFont(font);
    painter->setPen(Qt::black);
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);

    // draw level badge at top-right of card
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
    }

    // horizontal cooldown overlay (fills from right toward left)
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
