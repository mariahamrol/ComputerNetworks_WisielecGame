#include "mainwindow.h"
#include "startscreen.h"
#include "lobbyscreen.h"
#include "adminloginscreen.h"
#include "adminlobbyscreen.h"
#include "waitingroomscreen.h"
#include "gamescreen.h"
#include "gameendscreen.h"
#include "gamecontroller.h"
#include <QDebug>
#include <QMessageBox>

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
    gameEndScreen = new GameEndScreen(stack);

    controller = new GameController(this);

    stack->addWidget(startScreen);
    stack->addWidget(adminLoginScreen);
    stack->addWidget(lobbyScreen);
    stack->addWidget(adminLobbyScreen);
    stack->addWidget(waitingScreen);
    stack->addWidget(gameScreen);
    stack->addWidget(gameEndScreen);

    stack->setCurrentIndex(0);
    setCentralWidget(stack);

    connect(controller, &GameController::connectionSuccessful, this, [this]() {
        startScreen->isConnected = true;
        startScreen->hideConnectionError();
    });
    connect(controller, &GameController::connectionError, this, [this]() {
        startScreen->isConnected = false;
        startScreen->showConnectionError("Failed to connect to the server.");
    });
    connect(controller, &GameController::serverDisconnected, this, [this]() {
        qDebug() << "[MainWindow] Server disconnected - returning to start screen";
        stack->setCurrentWidget(startScreen);
        startScreen->isConnected = false;
        startScreen->showConnectionError("Server disconnected.");
    });
    connect(startScreen, &StartScreen::reconnectRequested,
        controller, &GameController::reconnectToServer);
    connect(startScreen, &StartScreen::closeApplicationRequested,
        this, &QMainWindow::close);
    
    // Connect when user clicks start - get server IP and connect, then send login
    connect(startScreen, &StartScreen::startClicked, this, [this]() {
        if (!startScreen->isConnected) {
            controller->connectToServerInitial(startScreen->getServerIp());
        }
    });
    
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
        controller->adminListUsersRequested();
    });
    connect(controller, &GameController::adminGamesListUpdated,
        adminLobbyScreen, &AdminLobbyScreen::displayGames);
    connect(controller, &GameController::adminUsersListUpdated,
        adminLobbyScreen, &AdminLobbyScreen::displayUsers);
    connect(controller, &GameController::adminGameDetailsUpdated,
        adminLobbyScreen, &AdminLobbyScreen::displayGameDetails);
    
    connect(adminLobbyScreen, &AdminLobbyScreen::terminateRequested,
        controller, &GameController::adminTerminateGameRequested);
    connect(adminLobbyScreen, &AdminLobbyScreen::viewRequested,
        controller, &GameController::adminGameDetailsRequested);
    
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
            this, [&](int /*gameId*/,
                QString hiddenWord,
                std::vector<QString> players,
                QString myNick)
            {
                qDebug() << "[MainWindow] gameStarted signal received!";
                qDebug() << "[MainWindow] Switching to GameScreen...";
                gameScreen->setPlayers(players, myNick);  // Reset isEliminated first
                gameScreen->setHiddenWord(hiddenWord);    // Then set word and reset keyboard

                stack->setCurrentWidget(gameScreen);
                qDebug() << "[MainWindow] Current widget set to GameScreen";
            });
    
    connect(gameScreen, &GameScreen::letterClicked,
            controller, &GameController::guessLetterRequested);
    
    connect(controller, &GameController::gameStateUpdated,
            this, [&](int /*gameId*/,
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
    
    // Handle start game failure
    connect(controller, &GameController::startGameFailed, this, [this]() {
        QMessageBox::warning(this, "Cannot Start Game", 
                           "Not enough players! You need at least 2 players to start the game.",
                           QMessageBox::Ok);
    });
    
    // Handle game ended with results
    connect(controller, &GameController::gameEnded, this, 
        [this](std::vector<QString> playerNames, std::vector<int> points, std::vector<bool> wasActive) {
        qDebug() << "[MainWindow] Game ended, showing results";
        
        std::vector<PlayerResult> results;
        for (size_t i = 0; i < playerNames.size(); ++i) {
            PlayerResult result;
            result.nick = playerNames[i];
            result.points = points[i];
            result.wasActive = wasActive[i];
            results.push_back(result);
        }
        
        gameEndScreen->displayResults(results);
        stack->setCurrentWidget(gameEndScreen);
    });
    
    // Return to lobby from game end screen
    connect(gameEndScreen, &GameEndScreen::returnToLobby, this, [this]() {
        stack->setCurrentWidget(lobbyScreen);
    });
}
