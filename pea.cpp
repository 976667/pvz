/*
 * =============================================================================
 * 文件: pea.cpp - 豌豆子弹实现文件
 * 描述: 实现Pea类的所有方法，包括构造函数、移动逻辑、碰撞检测和伤害计算
 *       包含天气系统对子弹速度的影响以及与僵尸的交互
 * =============================================================================
 */

#include "pea.h"
#include "zombie.h"
#include "weather.h"
#include "plant.h"
#include "commandmanager.h"
#include <QRandomGenerator>

/*
 * =============================================================================
 * 构造函数: Pea::Pea()
 *
 * 功能说明:
 *   初始化豌豆子弹的所有属性
 *   根据父植物计算攻击力倍率
 *
 * 参数:
 *   attack - 基础攻击力（默认0）
 *   flag - 是否为冰冻豌豆（默认false）
 *   parentPlant - 父植物指针（用于经验系统，默认nullptr）
 *
 * 速度计算:
 *   speed = 360.0 * 33 / 1000 ≈ 11.88 像素/帧
 *   360: 每秒360像素
 *   33: 游戏帧率（每33ms一帧）
 *   /1000: 转换为像素/帧
 *
 * 攻击力倍率:
 *   如果有父植物，使用父植物的等级计算倍率
 *   否则使用默认的1.0倍
 * =============================================================================
 */
Pea::Pea(int attack, bool flag, Plant *parentPlant)
{
    // 设置冰冻标志
    snow = flag;

    // 设置基础攻击力
    atk = attack;

    // 计算飞行速度: 每秒360像素，转换为每帧速度
    // 33ms * 360 / 1000 = 11.88 像素/帧
    speed = 360.0 * 33 / 1000;

    // 保存父植物指针
    m_parentPlant = parentPlant;

    // 根据父植物计算攻击力倍率
    if (m_parentPlant) {
        // 使用父植物的攻击力倍率
        m_attackMultiplier = m_parentPlant->getAttackMultiplier();
    } else {
        // 如果没有父植物，使用默认倍率
        m_attackMultiplier = 1.0;
    }
}

/*
 * =============================================================================
 * 方法: Pea::boundingRect() - 获取边界矩形
 *
 * 功能说明:
 *   返回子弹的边界矩形区域
 *   用于Qt图形系统的碰撞检测和绘制裁剪
 *
 * 返回值:
 *   QRectF(-12, -28, 24, 24)
 *   24x24像素的正方形区域
 *
 * 设计说明:
 *   中心点偏移(-12, -28)使得子弹视觉上更靠上
 *   适合从植物顶部发射的效果
 * =============================================================================
 */
QRectF Pea::boundingRect() const
{
    return QRectF(-12, -28, 24, 24);
}

/*
 * =============================================================================
 * 方法: Pea::paint() - 绘制豌豆子弹
 *
 * 功能说明:
 *   绘制豌豆子弹的图形
 *   根据snow标志选择不同的图片
 *
 * 图片资源:
 *   - snow=true: :/images/PeaSnow.png (冰冻豌豆)
 *   - snow=false: :/images/Pea.png (普通豌豆)
 *
 * 绘制说明:
 *   使用QPixmap::drawPixmap绘制图片
 *   绘制区域与boundingRect()一致
 * =============================================================================
 */
void Pea::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    // 根据类型选择不同的图片绘制
    // snow=true绘制冰冻豌豆，否则绘制普通豌豆
    painter->drawPixmap(QRect(-12, -28, 24, 24),
                       QPixmap(snow ? ":/images/PeaSnow.png" : ":/images/Pea.png"));
}

/*
 * =============================================================================
 * 方法: Pea::collidesWithItem() - 碰撞检测
 *
 * 功能说明:
 *   检测子弹是否与另一个图形项发生碰撞
 *   主要用于检测与僵尸的接触
 *
 * 碰撞条件（三者都满足才算碰撞）:
 *   1. other必须是Zombie类型
 *   2. y坐标必须相同（在同一行）
 *   3. x坐标距离小于15像素
 *
 * 与植物碰撞检测的区别:
 *   植物使用30像素的碰撞距离
 *   子弹使用15像素的碰撞距离
 *   这反映了子弹体积比植物小
 *
 * 返回值:
 *   true - 发生碰撞
 *   false - 未碰撞
 * =============================================================================
 */
bool Pea::collidesWithItem(const QGraphicsItem *other, Qt::ItemSelectionMode mode) const
{
    Q_UNUSED(mode)  // 未使用的参数

    // 检查类型、y坐标和x距离
    return other->type() == Zombie::Type && qFuzzyCompare(other->y(), y()) && qAbs(other->x() - x()) < 15;
}

/*
 * =============================================================================
 * 方法: Pea::advance() - 游戏每帧更新
 *
 * 功能说明:
 *   游戏每帧更新子弹状态的核心逻辑
 *   包括: 移动、碰撞检测、伤害计算、天气影响、经验系统
 *
 * 参数:
 *   phase - 前进阶段
 *     - phase=0: 仅更新动画
 *     - phase=1: 处理所有逻辑
 *
 * 处理流程:
 *   1. 阶段检查: 只在phase=1时处理
 *   2. 触发重绘: 调用update()
 *   3. 天气影响: 根据天气调整速度因子
 *   4. 碰撞检测: 检测与僵尸的碰撞
 *   5. 伤害结算: 对僵尸造成伤害
 *   6. 减速效果: 冰冻豌豆减速僵尸
 *   7. 经验传递: 给父植物增加经验
 *   8. 位置更新: 移动子弹
 *   9. 边界检查: 飞出屏幕则删除
 *
 * 天气速度因子表:
 *   - Wind: 1.3x (加速)
 *   - Rain: 1.0x (正常)
 *   - Clear: 1.0x (正常)
 *   - Snow: 0.75x (减速)
 *   - Sandstorm: 0.65x (严重减速)
 * =============================================================================
 */
void Pea::advance(int phase)
{
    // ========== 阶段检查 ==========
    // 只在phase=1时处理逻辑
    if (!phase)
        return;

    // 触发重绘
    update();

    // ========== 天气速度因子 ==========
    // 根据当前天气调整移动速度
    qreal factor = 1.0;

    switch (WeatherManager::instance()->current())
    {
    case Weather::Wind:
        // 大风天气: 加速30%
        factor = 1.3;
        break;
    case Weather::Rain:
        // 雨天: 正常速度
        factor = 1.0;
        break;
    case Weather::Snow:
        // 雪天: 减速25%
        factor = 0.75;
        break;
    case Weather::Sandstorm:
        // 沙尘暴: 减速35%
        factor = 0.65;
        break;
    case Weather::Clear:
    default:
        // 晴天: 正常速度
        factor = 1.0;
        break;
    }

    // ========== 碰撞检测 ==========
    // 获取所有与子弹碰撞的图形项
    QList<QGraphicsItem *> items = collidingItems();

    // 如果有碰撞发生
    if (!items.isEmpty())
    {
        // 从碰撞列表中随机选择一个僵尸
        // 使用QRandomGenerator确保随机性
        Zombie *zombie = qgraphicsitem_cast<Zombie *>(items[QRandomGenerator::global()->bounded(items.size())]);

        // ========== 伤害计算 ==========
        // 计算实际伤害 = 基础攻击力 * 父植物倍率
        int effectiveAtk = int(atk * m_attackMultiplier);

        // 对僵尸造成伤害
        zombie->hp -= effectiveAtk;

        // ========== 经验系统 ==========
        // 如果有父植物，给父植物增加经验
        if (m_parentPlant) {
            // 每次命中僵尸获得2点经验
            m_parentPlant->gainExperience(2);
        }

        // ========== 减速效果 ==========
        // 如果是冰冻豌豆且僵尸未被减速，则减速僵尸
        // 僵尸正常速度 > 0.55
        // 减速后速度 = 原速度 / 2
        if (snow && zombie->speed > 0.55)
            zombie->speed /= 2;

        // 碰撞后删除子弹
        delete this;
        return;
    }

    // ========== 位置更新 ==========
    // 根据天气因子和命令管理器速度倍率更新位置
    // 速度 = 基础速度 * 天气因子 * 游戏速度倍率
    setX(x() + speed * factor * CommandManager::instance()->speedMultiplier());

    // ========== 边界检查 ==========
    // 如果子弹飞出屏幕右侧（x > 1069），删除子弹
    if (x() > 1069)
        delete this;
}
