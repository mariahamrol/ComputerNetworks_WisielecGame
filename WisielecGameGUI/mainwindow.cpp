#include "mainwindow.h"
#include "startscreen.h"
#include "lobbyscreen.h"
#include "waitingroomscreen.h"
#include "gamescreen.h"
#include "gamecontroller.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Wisielec Game");
    resize(800, 800);

    stack = new QStackedWidget(this);
    //Start game screen only for entering login
    startScreen = new StartScreen(stack);
    //Waiting room screen our main lobby it's for choosing what game to join
    lobbyScreen = new LobbyScreen(stack);

    waitingScreen = new WaitingRoomScreen(stack);
    gameScreen = new GameScreen(stack);

    controller = new GameController(this);

    stack->addWidget(startScreen);
    stack->addWidget(lobbyScreen);
    stack->addWidget(waitingScreen);
    stack->addWidget(gameScreen);

    stack->setCurrentIndex(0);
    setCentralWidget(stack);

    connect(startScreen, &StartScreen::startClicked,
            controller, &GameController::loginRequested);

    // WaitingRoom → Controller
    connect(lobbyScreen, &LobbyScreen::joinGame,
            controller, &GameController::joinGameRequested);

    // Controller → GUI
    connect(controller, &GameController::loginAccepted, this, [&]() {
        stack->setCurrentWidget(lobbyScreen);
    });

    connect(controller, &GameController::lobbyStateUpdated,
            lobbyScreen, &LobbyScreen::display_games);

    connect(controller, &GameController::joinedGame,
            this, [&](int gameId,
                std::vector<QString> players,
                bool isHost)
            {
                waitingScreen->setRoomState(gameId, players, isHost);
                stack->setCurrentWidget(waitingScreen);
            });

    connect(waitingScreen, &WaitingRoomScreen::startGame,
            controller, &GameController::startGameRequested);

    connect(controller, &GameController::gameStarted,
            this, [&](int gameId,
                QString hiddenWord,
                std::vector<QString> players)
            {
                // gameScreen->setHiddenWord(hiddenWord);
                // gameScreen->setPlayers(players);

                stack->setCurrentWidget(gameScreen);
            });
}
