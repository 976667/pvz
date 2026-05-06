#include "map.h"
#include <QDebug>

Map::Map()
{
    dragOver = false;
    setAcceptDrops(true);
}

QRectF Map::boundingRect() const
{
    return QRectF(-369, -235, 738, 470);
}

void Map::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(widget)
    // painter->setPen(Qt::black);
    // painter->drawRect(boundingRect());
}

void Map::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if (event->mimeData()->hasText())
    {
        event->setAccepted(true);
        event->acceptProposedAction();
        dragOver = true;
        update();
    }
    else
        event->setAccepted(false);
}

void Map::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);
    dragOver = false;
    update();
}

void Map::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    dragOver = false;
    if (event->mimeData()->hasText())
    {
        QString s = event->mimeData()->text();
        qDebug() << "Map::dropEvent text:" << s << "pos:" << event->pos();
        QPointF pos = mapToScene(event->pos());
        pos.setX((int(pos.x()) - 249) / 82 * 82 + 290);
        pos.setY((int(pos.y()) - 81) / 98 * 98 + 130);
        // ensure computed pos is inside map bounds
        QPointF itemPos = mapFromScene(pos);
        if (!boundingRect().contains(itemPos)) {
            qDebug() << "Map::dropEvent computed pos outside map bounds:" << pos << "itemPos:" << itemPos;
            event->setAccepted(false);
            return;
        }
        event->acceptProposedAction();
        if (s == "Shovel")
        {
            QList<QGraphicsItem *> shopItems = scene()->items(QPointF(830, 15));
            if (!shopItems.empty())
            {
                Shovel *shovel = qgraphicsitem_cast<Shovel *>(shopItems[0]);
                if (shovel) {
                    // 延迟执行，避免在drag/drop回调中重入导致问题
                    Shovel *sp = shovel;
                    QPointF posCopy = pos;
                    QTimer::singleShot(std::chrono::milliseconds(0), [sp, posCopy]() {
                        // 仅在该item仍存在于scene中时执行
                        if (sp && sp->scene()) sp->removePlant(posCopy);
                    });
                } else
                    qDebug() << "Map::dropEvent shovel cast failed";
            } else {
                qDebug() << "Map::dropEvent no items at shovel location";
            }
        }
        else
        {
            QList<QGraphicsItem *> shopItems = scene()->items(QPointF(300, 15));
            if (!shopItems.empty())
            {
                Shop *shop = qgraphicsitem_cast<Shop *>(shopItems[0]);
                if (shop) {
                    // 延迟添加植物，避免在drop回调中直接修改scene造成重入问题
                    Shop *sp = shop;
                    QString sCopy = s;
                    QPointF posCopy = pos;
                    QTimer::singleShot(std::chrono::milliseconds(0), [sp, sCopy, posCopy]() {
                        if (sp && sp->scene()) sp->addPlant(sCopy, posCopy);
                    });
                } else
                    qDebug() << "Map::dropEvent shop cast failed";
            } else {
                qDebug() << "Map::dropEvent no items at shop location";
            }
        }
    }
    update();
}
