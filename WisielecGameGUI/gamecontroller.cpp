#include "gamecontroller.h"
#include "../client/ClientConnection.h"
#include <QDebug>
#include <iostream>
#include <QVariant>
#include <QString>

GameController::GameController(QObject *parent)
    : QObject(parent) {
        client = new ClientConnection();
        
        client->onServerShutdown = [this]() {
            qDebug() << "[GameController] Server shutdown detected";
            emit serverDisconnected();
        };
        // Setup lobby state callback - będzie aktywny zawsze
        client->onLobbyState = [this](const MsgLobbyState& msg) {
            qDebug() << "[GameController] Received lobby state update, games count:" << msg.games_count;
            std::vector<int> games;
            for (uint32_t i = 0; i < msg.games_count; ++i) {
                games.push_back(msg.games[i].game_id);
                qDebug() << "[GameController] Game ID:" << msg.games[i].game_id << "Players:" << msg.games[i].players_count;
            }
            emit lobbyStateUpdated(games);
            qDebug() << "[GameController] Emitted lobbyStateUpdated signal";
        };

        client->onStartGameOk = [this]() {
            qDebug() << "Game started successfully (host)";
        };

        // Callback dla game state - wywoływany dla wszystkich graczy gdy gra się rozpocznie
        client->onGameState = [this](const MsgGameState& msg) {
            qDebug() << "Game state received, game_id=" << msg.game_id;
            
            // Konwertuj dane graczy i ich życia
            std::vector<QString> qplayers;
            std::vector<int> lives;
            std::vector<int> points;
            bool gameJustStarted = true;
            
            // Pobierz ukryte słowo z serwera (z spacjami między literami)
            QString hiddenWord = QString::fromStdString(msg.word);
            QString displayWord = "";
            for (int i = 0; i < hiddenWord.length(); ++i) {
                if (i > 0) displayWord += " ";
                displayWord += hiddenWord[i];
            }
            hiddenWord = displayWord;
            
            // Pobierz zgadnięte litery globalnie (dla wszystkich graczy)
            QString guessedLetters = QString::fromStdString(msg.guessed_letters);
            qDebug() << "[GameController] Global guessed letters from server:" << guessedLetters;
            
            // Pobierz zgadnięte litery dla aktualnego gracza
            QString myGuessedLetters = "";
            for (uint8_t i = 0; i < msg.player_count && i < 8; ++i) {
                QString playerNick = QString::fromStdString(msg.players[i].nick);
                qplayers.push_back(playerNick);
                lives.push_back(msg.players[i].lives);
                points.push_back(msg.players[i].points);
                if (msg.players[i].lives != MAX_LIVES) {
                    gameJustStarted = false;
                }
                // Znajdź zgadnięte litery dla bieżącego gracza
                QString playerGuessed = QString::fromStdString(msg.players[i].guessed_letters);
                qDebug() << "[GameController] Player:" << playerNick << "guessed letters:" << playerGuessed;
                if (playerNick == myNickname) {
                    myGuessedLetters = playerGuessed;
                    qDebug() << "[GameController] Found my letters:" << myGuessedLetters;
                }
            }
            
            // If word_length is 0, we're still in waiting room - update player list
            if (msg.word_length == 0) {
                qDebug() << "[GameController] Game state in waiting room - updating player list";
                // Find owner (player with is_owner flag)
                QString owner = "";
                bool isHost = false;
                for (uint8_t i = 0; i < msg.player_count && i < 8; ++i) {
                    if (msg.players[i].is_owner) {
                        owner = QString::fromStdString(msg.players[i].nick);
                    }
                    if (QString::fromStdString(msg.players[i].nick) == myNickname && msg.players[i].is_owner) {
                        isHost = true;
                    }
                }
                emit joinedGame(msg.game_id, qplayers, owner, isHost);
            }
            else if (gameJustStarted && msg.word_length > 0) {
                emit gameStarted(msg.game_id, hiddenWord, qplayers, myNickname);
            }
            
            if (msg.word_length > 0) {
                emit gameStateUpdated(msg.game_id, hiddenWord, qplayers, lives, points, guessedLetters, myGuessedLetters);
            }
        };

        client->onPlayerEliminated = [this]() {
            qDebug() << "Player eliminated from game";
            emit playerEliminated();
        };
        
        client->onStartGameFail = [this]() {
            qDebug() << "Start game failed - not enough players";
            emit startGameFailed();
        };
        
        client->onGameResults = [this](const MsgGameResults& results) {
            qDebug() << "Game results received";
            std::vector<QString> playerNames;
            std::vector<int> points;
            std::vector<bool> wasActive;
            
            for (uint8_t i = 0; i < results.player_count; ++i) {
                playerNames.push_back(QString::fromStdString(results.players[i].nick));
                points.push_back(results.players[i].points);
                wasActive.push_back(results.players[i].was_active != 0);
            }
            
            emit gameEnded(playerNames, points, wasActive);
        };
        
        client->onGameEnd = [this]() {
            qDebug() << "Game ended (room closed or host left)";
            emit roomClosed();
        };

        // Admin callbacks
        client->onAdminPasswordRequired = [this]() {
            qDebug() << "Admin password required";
            emit adminPasswordRequired();
        };
        client->onAdminLoginOk = [this]() {
            qDebug() << "Admin login OK";
            emit adminLoginOk();
        };
        client->onAdminLoginFail = [this]() {
            qDebug() << "Admin login FAIL";
            emit adminLoginFail();
        };
        client->onAdminGamesList = [this](const MsgAdminGamesList& msg) {
            qDebug() << "Received admin games list, games count:" << msg.games_count;
            emit adminGamesListUpdated(msg);
        };
        client->onAdminUsersList = [this](const MsgAdminUsersList& msg) {
            qDebug() << "Received admin users list, users count:" << msg.users_count;
            emit adminUsersListUpdated(msg);
        };
        client->onAdminGameDetails = [this](const MsgAdminGameDetails& msg) {
            qDebug() << "Received admin game details for game:" << msg.game_id;
            emit adminGameDetailsUpdated(msg);
        };
        client->onAdminTerminateOk = [this]() {
            qDebug() << "Admin terminate OK";
            emit adminTerminateOk();
        };
        client->onAdminTerminateFail = [this]() {
            qDebug() << "Admin terminate FAIL";
            emit adminTerminateFail();
        };
        
    }                           

void GameController::loginRequested(const QString &login)
{
    if (login.isEmpty()) {
        emit loginRejected("Empty login");
        return;
    }
    client->login(login.toStdString());

    client->onLoginOk = [this, login]() {
        qDebug() << "Login successful from server";
        myNickname = login;
        emit loginAccepted(login);
    };
    client->onLoginTaken = [this]() {
        qDebug() << "Login taken reported by server";
        emit loginRejected("Login already taken");
    };
}

void GameController::joinGameRequested(int gameId)
{
    client->onJoinRoomOk =
        [this](uint32_t gid, uint8_t, const std::string &owner, const std::vector<std::string> &players)
    {
        qDebug() << "Joined game successfully, id=" << (int)gid;
        std::vector<QString> qplayers;
        for (const auto &p : players)
            qplayers.push_back(QString::fromStdString(p));
        QString qOwner = QString::fromStdString(owner);

        bool isHost = false;

        emit joinedGame(gid, qplayers, qOwner, isHost);
        };

    client->onError = [](const std::string &reason) {
        qDebug() << "Error joining game:" << QString::fromStdString(reason);
    };

    client->joinRoom(gameId);
};

void GameController::createGameRequested()
{
    qDebug() << "Creating game requested";
    client->onCreateRoomOk = [this](uint32_t gameId, uint8_t, const std::string &owner, const std::vector<std::string> &players) {
        qDebug() << "Game created successfully, id=" << (int)gameId;
        std::vector<QString> qplayers;
        for (const auto &p : players) qplayers.push_back(QString::fromStdString(p));
        QString qOwner = QString::fromStdString(owner);
        bool isHost = true;
        emit createdGame(gameId);
        emit joinedGame(gameId, qplayers, qOwner, isHost);
    };
    client->onJoinRoomOk =
        [this](uint32_t gid, uint8_t, const std::string &owner, const std::vector<std::string> &players)
    {
        std::vector<QString> qplayers;
        for (const auto &p : players)
            qplayers.push_back(QString::fromStdString(p));
        QString qOwner = QString::fromStdString(owner);

        bool isHost = true;

        emit joinedGame(gid, qplayers, qOwner, isHost);
    };

    client->onError = [this](const std::string &reason) {
        qDebug() << "Error creating game:" << QString::fromStdString(reason);
    };
    client->createRoom();
}

void GameController::startGameRequested(int gameId)
{
    qDebug() << "Starting game requested for game ID:" << gameId;
    client->onError = [this](const std::string &reason) {
        qDebug() << "Error starting game:" << QString::fromStdString(reason);
    };
    client->startGame(gameId);
}

void GameController::guessLetterRequested(QChar letter)
{
    qDebug() << "Guessing letter:" << letter;
    client->onGuessLetterOk = [this, letter]() {
        qDebug() << "Letter guessed successfully:" << letter;
    };
    client->onGuessLetterFail = [this, letter]() {
        qDebug() << "Failed to guess letter:" << letter;
    };
    client->onError = [this](const std::string &reason) {
        qDebug() << "Error guessing letter:" << QString::fromStdString(reason);
    };
    client->guessLetter(letter.toLower().toLatin1());
}

void GameController::exitGameRequested()
{
    qDebug() << "Exit game requested";
    client->onExitGameOk = [this]() {
        qDebug() << "Exited game successfully";
        emit exitedGame();
    };
    client->onExitGameFail = [this]() {
        qDebug() << "Failed to exit game";
    };
    client->exitGame();
}

void GameController::exitRoomRequested()
{
    qDebug() << "Exit room requested";
    client->onExitRoomOk = [this]() {
        qDebug() << "Exited room successfully";
        emit exitedRoom();
    };
    client->onExitRoomFail = [this]() {
        qDebug() << "Failed to exit room";
    };
    client->exitRoom();
}

void GameController::adminLoginRequested(const QString &password) {
    qDebug() << "Admin login requested";
    client->adminLogin(password.toStdString());
}

void GameController::adminListGamesRequested() {
    qDebug() << "Admin list games requested";
    client->adminListGames();
}

void GameController::adminListUsersRequested() {
    qDebug() << "Admin list users requested";
    client->adminListUsers();
}

void GameController::adminGameDetailsRequested(int gameId) {
    qDebug() << "Admin game details requested for game ID:" << gameId;
    client->adminGetGameDetails((uint32_t)gameId);
}

void GameController::adminTerminateGameRequested(int gameId) {
    qDebug() << "Admin terminate game requested for game ID:" << gameId;
    client->onError = [this](const std::string &reason) {
        qDebug() << "Error terminating game:" << QString::fromStdString(reason);
    };
    client->onAdminTerminateOk = [this]() {
        qDebug() << "Game terminated successfully";
    };
    client->onAdminTerminateFail = [this]() {
        qDebug() << "Failed to terminate game";
    };
    client->adminTerminateGame((uint32_t)gameId);
}

void GameController::reconnectToServer()
{
    qDebug() << "[GameController] Attempting to reconnect to server...";
    if(client->connectToServer("127.0.0.1", 12345)) {
        emit connectionSuccessful();
        qDebug() << "[GameController] Reconnected to server successfully";
    } else {
        emit connectionError();
        qDebug() << "[GameController] Failed to reconnect to server";
    }
}

void GameController::connectToServerInitial()
{
    qDebug() << "[GameController] Initial connection attempt...";
    if(!client->connectToServer("127.0.0.1", 12345)) {
        emit connectionError();
        qDebug() << "[GameController] Error connecting to server";
    } 
    else {
        emit connectionSuccessful();
        qDebug() << "[GameController] Connected to server successfully";
    }
}
