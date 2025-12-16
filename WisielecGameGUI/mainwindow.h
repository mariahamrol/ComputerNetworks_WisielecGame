#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include "gamecontroller.h"

class StartScreen;
class LobbyScreen;
class WaitingRoomScreen;
class GameScreen;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    QStackedWidget *stack;
    StartScreen *startScreen;
    LobbyScreen *lobbyScreen;
    WaitingRoomScreen *waitingScreen;
    GameController *controller;
    GameScreen *gameScreen;
};

#endif
