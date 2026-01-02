#include "mainwindow.h"
#include "startscreen.h"
#include "lobbyscreen.h"
#include "adminloginscreen.h"
#include "adminlobbyscreen.h"
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
    adminLoginScreen = new AdminLoginScreen(stack); // Changed to member variable
    //Waiting room screen our main lobby it's for choosing what game to join
    lobbyScreen = new LobbyScreen(stack);
    adminLobbyScreen = new AdminLobbyScreen(stack); // Changed to member variable

    waitingScreen = new WaitingRoomScreen(stack);
    gameScreen = new GameScreen(stack);

    controller = new GameController(this);

    stack->addWidget(startScreen);
    stack->addWidget(adminLoginScreen);
    stack->addWidget(lobbyScreen);
    stack->addWidget(adminLobbyScreen);
    stack->addWidget(waitingScreen);
    stack->addWidget(gameScreen);

    stack->setCurrentIndex(0);
    setCentralWidget(stack);

        connect(startScreen, &StartScreen::startClicked,
            controller, &GameController::loginRequested);
    // Admin: prompt for password when server requests
    connect(controller, &GameController::adminPasswordRequired, this, [this]() {
        stack->setCurrentWidget(adminLoginScreen);
    });
    connect(adminLoginScreen, &AdminLoginScreen::submitPassword,
        controller, &GameController::adminLoginRequested);
    // adminLoginFail has no arguments; adapt with a lambda to show a message
    connect(controller, &GameController::adminLoginFail, this, [this]() {
        adminLoginScreen->showError("Invalid admin password");
    });
    connect(controller, &GameController::adminLoginOk, this, [this]() {
        stack->setCurrentWidget(adminLobbyScreen);
        controller->adminListGamesRequested();
    });
    connect(controller, &GameController::adminGamesListUpdated,
        adminLobbyScreen, &AdminLobbyScreen::displayGames);
    connect(adminLobbyScreen, &AdminLobbyScreen::refreshRequested,
        controller, &GameController::adminListGamesRequested);
    connect(adminLobbyScreen, &AdminLobbyScreen::terminateRequested,
        controller, &GameController::adminTerminateGameRequested);
    // TODO: viewRequested could open a read-only game view; for now, refresh listing
    connect(adminLobbyScreen, &AdminLobbyScreen::viewRequested,
        controller, &GameController::adminListGamesRequested);

    // WaitingRoom → Controller
    connect(lobbyScreen, &LobbyScreen::joinGame,
            controller, &GameController::joinGameRequested);

    // LobbyScreen → Controller
    connect(lobbyScreen, &LobbyScreen::createGame,
            controller, &GameController::createGameRequested);

    // Controller → GUI
    connect(controller, &GameController::loginAccepted, this, [&](const QString &login){
        stack->setCurrentWidget(lobbyScreen);
        lobbyScreen->set_login(login);
    });

    // Show error and return to start screen when login rejected
    connect(controller, &GameController::loginRejected, this, [&](const QString &reason){
        stack->setCurrentWidget(startScreen);
        startScreen->showLoginError(reason);
    });

    connect(controller, &GameController::lobbyStateUpdated,
            lobbyScreen, &LobbyScreen::display_games);


    connect(controller, &GameController::joinedGame,
            this, [&](int gameId,
                std::vector<QString> players,
                QString owner,
                bool isHost)
            {
                waitingScreen->setRoomState(gameId, players, owner, isHost);
                stack->setCurrentWidget(waitingScreen);
            });

    connect(waitingScreen, &WaitingRoomScreen::startGame,
            controller, &GameController::startGameRequested);

    connect(controller, &GameController::gameStarted,
            this, [&](int gameId,
                QString hiddenWord,
                std::vector<QString> players,
                QString myNick)
            {
                qDebug() << "[MainWindow] gameStarted signal received!";
                qDebug() << "[MainWindow] Switching to GameScreen...";
                gameScreen->setHiddenWord(hiddenWord);
                gameScreen->setPlayers(players, myNick);

                stack->setCurrentWidget(gameScreen);
                qDebug() << "[MainWindow] Current widget set to GameScreen";
            });
    
    connect(gameScreen, &GameScreen::letterClicked,
            controller, &GameController::guessLetterRequested);
    
    connect(controller, &GameController::gameStateUpdated,
            this, [&](int gameId,
                QString hiddenWord,
                std::vector<QString> players,
                std::vector<int> lives,
                std::vector<int> points,
                QString guessedLetters,
                QString myGuessedLetters)
            {
                gameScreen->updateGameState(hiddenWord, players, lives, points, guessedLetters, myGuessedLetters);
            });
    
    connect(controller, &GameController::wrongLetterGuessed,
            gameScreen, &GameScreen::incrementMyMistakes);

    connect(controller, &GameController::playerEliminated,
            gameScreen, &GameScreen::disablePlayer);
    
    // Exit game/room connections
    connect(gameScreen, &GameScreen::exitGame,
            controller, &GameController::exitGameRequested);
    
    connect(waitingScreen, &WaitingRoomScreen::exitRoom,
            controller, &GameController::exitRoomRequested);
    
    connect(controller, &GameController::exitedGame, this, [&]() {
        qDebug() << "[MainWindow] Exited game, returning to lobby";
        stack->setCurrentWidget(lobbyScreen);
    });
    
    connect(controller, &GameController::exitedRoom, this, [&]() {
        qDebug() << "[MainWindow] Exited room, returning to lobby";
        stack->setCurrentWidget(lobbyScreen);
    });
    
    connect(controller, &GameController::roomClosed, this, [&]() {
        qDebug() << "[MainWindow] Room closed (host left), returning to lobby";
        stack->setCurrentWidget(lobbyScreen);
    });
}
