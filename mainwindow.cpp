#include "mainwindow.h"
#include <QCoreApplication>
#include <QRandomGenerator>
#include <QUrl>
#include <QGraphicsTextItem>
#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QSvgRenderer>
#include "commandmanager.h"
#include "weathereffect.h"

static QPixmap renderWeatherIcon(const QString &resourcePath, int size = 40)
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    QSvgRenderer renderer(resourcePath);
    
    // 确保SVG正确加载
    if (!renderer.isValid()) {
        qWarning() << "Failed to load SVG:" << resourcePath;
        return pixmap;  // 返回透明pixmap
    }
    
    QPainter painter(&pixmap);
    renderer.render(&painter);
    painter.end();
    return pixmap;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    sound = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    sound->setAudioOutput(audioOutput);
    audioOutput->setVolume(0.6f);
    sound->setLoops(QMediaPlayer::Infinite);
    sound->setSource(QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/Grazy Dave.mp3"));
    timer = new QTimer;
    scene = new QGraphicsScene(this);
    scene->setSceneRect(150, 0, 900, 600);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    shop = new Shop;
    shop->setPos(520, 45);
    scene->addItem(shop);
    Shovel *shovel = new Shovel;
    shovel->setPos(830, 40);
    scene->addItem(shovel);
    Button *button = new Button(sound, timer);
    button->setPos(970, 20);
    scene->addItem(button);
    Map *map = new Map;
    map->setPos(618, 326);
    scene->addItem(map);
    for (int i = 0; i < 5; ++i)
    {
        Mower *mower = new Mower;
        mower->setPos(210, 130 + 98 * i);
        scene->addItem(mower);
    }
    view = new QGraphicsView(scene, this);
    view->resize(902, 602);
    view->setRenderHint(QPainter::Antialiasing);
    originalBackground = QPixmap(":/images/Background.jpg");
    view->setBackgroundBrush(originalBackground);
    view->setCacheMode(QGraphicsView::CacheBackground);
    view->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    connect(timer, &QTimer::timeout, scene, &QGraphicsScene::advance);
    connect(timer, &QTimer::timeout, this, &MainWindow::addZombie);
    connect(timer, &QTimer::timeout, this, &MainWindow::check);
    
    // 创建天气状态栏背景（半透明矩形）
    QGraphicsRectItem *weatherBg = scene->addRect(168, 8, 58, 48, QPen(Qt::NoPen), 
                                                   QBrush(QColor(0, 0, 0, 120)));
    weatherBg->setZValue(15);

    weatherIcon = scene->addPixmap(renderWeatherIcon(":/images/weather_clear.svg", 40));
    weatherIcon->setPos(170, 10);
    weatherIcon->setZValue(20);
    
    WeatherManager *weatherMgr = WeatherManager::instance();
    // 同步天气切换到游戏主定时器（使天气随倍速/暂停同步）
    weatherMgr->syncToGameTimer(timer, 33);
    // 同步命令/卡牌冷却到游戏主定时器（使卡牌冷却随倍速/暂停同步）
    CommandManager::instance()->syncToGameTimer(timer, 33);
    // 持续根据游戏主定时器生成天气粒子（随倍速/暂停同步）
    connect(timer, &QTimer::timeout, this, [this]() {
        Weather w = WeatherManager::instance()->current();
        int r = QRandomGenerator::global()->bounded(100);
        switch (w) {
            case Weather::Rain:
                if (r < 40) createRainEffect(1);
                break;
            case Weather::Snow:
                if (r < 25) createSnowEffect(1);
                break;
            case Weather::Wind:
                if (r < 30) createWindEffect(1);
                break;
            case Weather::Sandstorm:
                if (r < 60) createSandstormEffect(1);
                break;
            default:
                break;
        }
    });

    connect(weatherMgr, &WeatherManager::weatherChanged, this, [this](Weather w) {
        QString text;
        QPixmap bg = originalBackground;
        QPainter painter(&bg);
        clearWeatherEffects();
        
        switch(w) {
            case Weather::Clear:
                text = "Weather: CLEAR";
                weatherIcon->setPixmap(renderWeatherIcon(":/images/weather_clear.svg", 40));
                break;
            case Weather::Rain:
                text = "Weather: RAIN";
                weatherIcon->setPixmap(renderWeatherIcon(":/images/weather_rain.svg", 40));
                painter.fillRect(bg.rect(), QColor(100, 150, 200, 80));
                createRainEffect(20);
                break;
            case Weather::Snow:
                text = "Weather: SNOW";
                weatherIcon->setPixmap(renderWeatherIcon(":/images/weather_snow.svg", 40));
                painter.fillRect(bg.rect(), QColor(220, 240, 255, 60));
                createSnowEffect(12);
                break;
            case Weather::Wind:
                text = "Weather: WIND";
                weatherIcon->setPixmap(renderWeatherIcon(":/images/weather_wind.svg", 40));
                painter.fillRect(bg.rect(), QColor(200, 200, 200, 40));
                createWindEffect(10);
                break;
            case Weather::Sandstorm:
                text = "Weather: SANDSTORM";
                weatherIcon->setPixmap(renderWeatherIcon(":/images/weather_sandstorm.svg", 40));
                painter.fillRect(bg.rect(), QColor(220, 180, 100, 100));
                createSandstormEffect(30);
                break;
        }
        painter.end();
        view->setBackgroundBrush(bg);
    });
    weatherMgr->start(15000);
    
    sound->play();
    timer->start(33);
    view->show();

    // Command card system (quick test): add default cards and a compact graphical card
    CommandManager::instance()->addDefaultCards();
    // create compact CommandCardItem instances for default command cards
    int x = 220;
    const int spacing = 120;
    const QVector<QString> defaultIds = {"instant_sun", "speed_boost"};
    for (const QString &id : defaultIds) {
        int cd = CommandManager::instance()->cooldownMsFor(id);
        if (cd <= 0) cd = 5000;
        QString text;
        if (id == "instant_sun") text = "Sun Card";
        else if (id == "speed_boost") text = "Speed Boost";
        
        else text = id;
        CommandCardItem *item = new CommandCardItem(id, text, cd, nullptr);
        item->setPos(x, 560);
        scene->addItem(item);
        item->setZValue(200);
        x += spacing;
    }
    // 同步命令管理器到游戏主计时器并连接效果处理
    CommandManager::instance()->syncToGameTimer(timer, 33);
    connect(CommandManager::instance(), &CommandManager::cardUsed, this, &MainWindow::handleCardUsed);
    connect(timer, &QTimer::timeout, this, &MainWindow::onGameTick);

}

void MainWindow::handleCardUsed(const QString &id, const QVariantMap &params)
{
    if (id == "instant_sun") {
        return;
    }
    qint64 now = CommandManager::instance()->currentGameMs();
    if (id == "speed_boost") {
        int multiplier = params.value("multiplier", 2).toInt();
        int duration = params.value("durationMs", 5000).toInt();
        QVariantMap effect;
        effect["id"] = id;
        // endMs is in game-ms; advancing game-ms already accounts for user and effect multipliers
        effect["endMs"] = now + qint64(duration);
        // store previous effect multiplier (do not overwrite user multiplier)
        effect["prevEffectMultiplier"] = CommandManager::instance()->effectSpeedMultiplier();
        double newEffectMult = CommandManager::instance()->effectSpeedMultiplier() * multiplier;
        CommandManager::instance()->setEffectSpeedMultiplier(newEffectMult);
        m_activeEffects.append(effect);
    }
}

void MainWindow::onGameTick()
{
    qint64 now = CommandManager::instance()->currentGameMs();
    for (int i = m_activeEffects.size() - 1; i >= 0; --i) {
        QVariantMap effect = m_activeEffects.at(i);
        QString id = effect.value("id").toString();
        qint64 endMs = effect.value("endMs").toLongLong();
        if (now >= endMs) {
            if (id == "speed_boost") {
                double prevEffect = effect.value("prevEffectMultiplier", 1.0).toDouble();
                CommandManager::instance()->setEffectSpeedMultiplier(prevEffect);
            }
            m_activeEffects.removeAt(i);
        }
    }
}

MainWindow::~MainWindow()
{
    delete audioOutput;
    delete sound;
    delete timer;
    delete scene;
    delete view;
}

void MainWindow::addZombie()
{
    static int low = 4;
    static int high = 8;
    static int maxtime = 20 * 1000 / 33;
    static int time = maxtime / 2;
    static int counter = 0;
    static int gameFrame = 0;
    gameFrame++;
    
    int bucketFrame = 60 * 1000 / 33;
    int screenFrame = 2 * 60 * 1000 / 33;
    int footballFrame = 3 * 60 * 1000 / 33;
    
    if (++counter >= time)
    {
        if (++low > high)
        {
            maxtime /= 1.3;
            high *= 2;
        }
        counter = 0;
        time = QRandomGenerator::global()->bounded(2 * maxtime / 3) + maxtime / 3;

        int i = QRandomGenerator::global()->bounded(5);
        Zombie *zombie;
        int rand = QRandomGenerator::global()->bounded(100);

        if (gameFrame < bucketFrame) {
            if (rand < 60)
                zombie = new ConeZombie;
            else
                zombie = new BasicZombie;
        } else if (gameFrame < screenFrame) {
            if (rand < 10)
                zombie = new BucketZombie;
            else if (rand < 60)
                zombie = new ConeZombie;
            else
                zombie = new BasicZombie;
        } else if (gameFrame < footballFrame) {
            if (rand < 10)
                zombie = new ScreenZombie;
            else if (rand < 20)
                zombie = new BucketZombie;
            else if (rand < 60)
                zombie = new ConeZombie;
            else
                zombie = new BasicZombie;
        } else {
            if (rand < 5)
                zombie = new FootballZombie;
            else if (rand < 15)
                zombie = new ScreenZombie;
            else if (rand < 25)
                zombie = new BucketZombie;
            else if (rand < 60)
                zombie = new ConeZombie;
            else
                zombie = new BasicZombie;
        }

        zombie->setPos(1028, 130 + 98 * i);
        scene->addItem(zombie);
    }
}

void MainWindow::check()
{
    static int time = 1 * 1000 / 33;
    static int counter = 0;
    if (++counter >= time)
    {
        counter = 0;
        const QList<QGraphicsItem *> items = scene->items();
        foreach (QGraphicsItem *item, items)
            if (item->type() == Zombie::Type && item->x() < 200)
            {
                scene->addPixmap(QPixmap(":/images/ZombiesWon.png"))->setPos(336, 92);
                scene->advance();
                timer->stop();
                return;
            }
    }
}

void MainWindow::clearWeatherEffects()
{
    // 先收集所有Particle类型的items到list中，然后再删除
    QList<Particle *> particles;
    const QList<QGraphicsItem *> items = scene->items();
    foreach (QGraphicsItem *item, items) {
        Particle *p = dynamic_cast<Particle *>(item);
        if (p) {
            particles.append(p);
        }
    }
    
    // 删除收集到的particles
    foreach (Particle *p, particles) {
        scene->removeItem(p);
        delete p;
    }
}

void MainWindow::createRainEffect(int count)
{
    for (int i = 0; i < count; ++i) {
        qreal x = QRandomGenerator::global()->bounded(1050);
        qreal y = QRandomGenerator::global()->bounded(600) - 100;
        Particle *raindrop = new Particle(x, y, Particle::Raindrop);
        scene->addItem(raindrop);
        raindrop->setZValue(100);
    }
}

void MainWindow::createSnowEffect(int count)
{
    for (int i = 0; i < count; ++i) {
        qreal x = QRandomGenerator::global()->bounded(1050);
        qreal y = QRandomGenerator::global()->bounded(600) - 100;
        Particle *snowflake = new Particle(x, y, Particle::Snowflake);
        scene->addItem(snowflake);
        snowflake->setZValue(100);
    }
}

void MainWindow::createWindEffect(int count)
{
    for (int i = 0; i < count; ++i) {
        qreal x = QRandomGenerator::global()->bounded(1050);
        qreal y = QRandomGenerator::global()->bounded(600);
        Particle *windParticle = new Particle(x, y, Particle::WindParticle);
        scene->addItem(windParticle);
        windParticle->setZValue(100);
    }
}

void MainWindow::createSandstormEffect(int count)
{
    for (int i = 0; i < count; ++i) {
        qreal x = QRandomGenerator::global()->bounded(1050);
        qreal y = QRandomGenerator::global()->bounded(600) - 50;
        Particle *sandstorm = new Particle(x, y, Particle::SandstormParticle);
        scene->addItem(sandstorm);
        sandstorm->setZValue(100);
    }
}
