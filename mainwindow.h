#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QTimer>
#include <QPushButton>
#include <QList>
#include <QVariantMap>
#include <QGraphicsPixmapItem>
#include "shop.h"
#include "card.h"
#include "weather.h"
#include "weathereffect.h"
#include "commandcarditem.h"

class Particle;
#include "shovel.h"
#include "button.h"
#include "map.h"
#include "mower.h"
#include "basiczombie.h"
#include "conezombie.h"
#include "bucketzombie.h"
#include "screenzombie.h"
#include "footballzombie.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    void addZombie();
    void check();
private slots:
    void handleCardUsed(const QString &id, const QVariantMap &params);
    void onGameTick();
private:
    QMediaPlayer *sound;
    QAudioOutput *audioOutput;
    QTimer *timer;
    QGraphicsScene *scene;
    QGraphicsView *view;
    Shop *shop;
    CommandCardItem *cmdCardItem;
    QGraphicsTextItem *weatherDisplay;
    QGraphicsPixmapItem *weatherIcon;
    QPixmap originalBackground;
    QList<QVariantMap> m_activeEffects;
    
    void clearWeatherEffects();
    void createRainEffect(int count = 30);
    void createSnowEffect(int count = 20);
    void createWindEffect(int count = 25);
    void createSandstormEffect(int count = 40);
    void onCardClicked();
    void updateCardButton();
};

#endif // MAINWINDOW_H
