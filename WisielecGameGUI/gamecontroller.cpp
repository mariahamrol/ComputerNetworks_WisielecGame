#include "gamecontroller.h"
#include "../client/ClientConnection.h"
#include <QDebug>
#include <iostream>
#include <QVariant>
#include <QString>

GameController::GameController(QObject *parent)
    : QObject(parent) {
        client = new ClientConnection();
        client->connectToServer("127.0.0.1", 12345);
        
        // Setup lobby state callback - będzie aktywny zawsze
        client->onLobbyState = [this](const MsgLobbyState& msg) {
            std::vector<int> games;
            for (uint32_t i = 0; i < msg.games_count; ++i) {
                games.push_back(msg.games[i].game_id);
            }
            emit lobbyStateUpdated(games);
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
            bool gameJustStarted = true;
            
            // Pobierz ukryte słowo z serwera (z spacjami między literami)
            QString hiddenWord = QString::fromStdString(msg.word);
            QString displayWord = "";
            for (int i = 0; i < hiddenWord.length(); ++i) {
                if (i > 0) displayWord += " ";
                displayWord += hiddenWord[i];
            }
            hiddenWord = displayWord;
            
            // Pobierz zgadnięte litery
            QString guessedLetters = QString::fromStdString(msg.guessed_letters);
            
            for (uint8_t i = 0; i < msg.player_count && i < 8; ++i) {
                qplayers.push_back(QString::fromStdString(msg.players[i].nick));
                lives.push_back(msg.players[i].lives);
                if (msg.players[i].lives != MAX_LIVES) {
                    gameJustStarted = false;
                }
            }
            
            // Jeśli to początek gry, wyślij gameStarted, w przeciwnym razie gameStateUpdated
            if (gameJustStarted) {
                emit gameStarted(msg.game_id, hiddenWord, qplayers, myNickname);
            } else {
                emit gameStateUpdated(msg.game_id, hiddenWord, qplayers, lives, guessedLetters);
            }
        };
    }                           

void GameController::loginRequested(const QString &login)
{
    // 1. CHECK if login is valid
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
        [this](uint32_t gid,uint8_t playersCount,const std::string &owner,const std::vector<std::string> &players)
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
    client->onCreateRoomOk = [this](uint32_t gameId, uint8_t playersCount, const std::string &owner, const std::vector<std::string> &players) {
        qDebug() << "Game created successfully, id=" << (int)gameId;
        std::vector<QString> qplayers;
        for (const auto &p : players) qplayers.push_back(QString::fromStdString(p));
        QString qOwner = QString::fromStdString(owner);
        bool isHost = true;
        emit createdGame(gameId);
        emit joinedGame(gameId, qplayers, qOwner, isHost);
    };
    client->onJoinRoomOk =
        [this](uint32_t gid, uint8_t playersCount, const std::string &owner, const std::vector<std::string> &players)
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
