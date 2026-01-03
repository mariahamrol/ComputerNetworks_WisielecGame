#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include "gamecontroller.h"

class AdminLoginScreen;
class AdminLobbyScreen;
class StartScreen;
class LobbyScreen;
class WaitingRoomScreen;
class GameScreen;
class GameEndScreen;

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
    GameEndScreen *gameEndScreen;
    AdminLoginScreen *adminLoginScreen; // Added member variable
    AdminLobbyScreen *adminLobbyScreen; // Added member variable
};

#endif
