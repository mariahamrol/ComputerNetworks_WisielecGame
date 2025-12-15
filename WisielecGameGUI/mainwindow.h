#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>

class StartScreen;
class WaitingRoomScreen;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    QStackedWidget *stack;
    StartScreen *startScreen;
    WaitingRoomScreen *waitingScreen;
};

#endif
