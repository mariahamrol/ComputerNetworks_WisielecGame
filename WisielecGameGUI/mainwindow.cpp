#include "mainwindow.h"
#include "startscreen.h"
#include "waitingroomscreen.h"
#include <vector>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Wisielec Game");
    resize(800, 800);

    stack = new QStackedWidget(this);
    //Start game screen only for entering login
    startScreen = new StartScreen(stack);
    //Waiting room screen our main lobby it's for choosing what game to join
    waitingScreen = new WaitingRoomScreen(stack);

    stack->addWidget(startScreen);
    stack->addWidget(waitingScreen);
    //Later this part should be done by getting a message from server and dinamicly changing valid games so this
    std::vector<int> games;
    for (int i = 1; i <= 15; ++i){
        games.push_back(i);
    }

    stack->setCurrentIndex(0);
    setCentralWidget(stack);

    //We open Waiting Room only if server sent a message that login is correct
    //If loghin was incorrect we stay in startScreen and error appears? or maybe just a warning i dont know but it doesnt matter
    connect(startScreen, &StartScreen::startClicked, this, [=](const char *login) {
        waitingScreen->set_login(login);
        waitingScreen->set_games(games);
        stack->setCurrentWidget(waitingScreen);
    });
}
