#include "gamecontroller.h"
#include "../client/ClientConnection.h"
#include <QDebug>

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
    }

void GameController::loginRequested(const QString &login)
{
    // 1. CHECK if login is valid
    if (login.isEmpty()) {
        emit loginRejected("Empty login");
        return;
    }
    client->login(login.toStdString());

    client->onLoginOk = [this, &login]() {
        qDebug() << "Login successful from server";
        emit loginAccepted(login);
    };
    client->onLoginTaken = [this]() {
        qDebug() << "Login taken reported by server";
        emit loginRejected("Login already taken");
    };
}

void GameController::joinGameRequested(int gameId)
{
    // register callback before sending request so we can validate returned id
    client->onJoinRoomOk = [this, gameId](uint32_t returnedId, uint8_t playersCount, const std::string &owner, const std::vector<std::string> &players) {
        qDebug() << "Joined game response received, returnedId=" << (int)returnedId;
        if ((int)returnedId != gameId) {
            qDebug() << "Mismatch in joined game id! requested=" << gameId << " got=" << (int)returnedId;
            return; // ignore or handle error
        }
        // Convert players to QString vector
        std::vector<QString> qplayers;
        for (const auto &p : players) qplayers.push_back(QString::fromStdString(p));
        // emit joinedGame to switch UI
        emit joinedGame(gameId, qplayers, false);
    };
    client->onError = [this](const std::string &reason) {
        qDebug() << "Error joining game:" << QString::fromStdString(reason);
    };
    client->joinRoom(gameId);
    qDebug() << "Joining game" << gameId;
}

void GameController::startGameRequested(int gameId)
{
    qDebug() << "Starting game" << gameId;

    // MOCK – later this comes from server
    QString hiddenWord = "_ _ _ _ _";
    std::vector<QString> players = {
        "alice",
        "bob",
        "you"
    };

    emit gameStarted(gameId, hiddenWord, players);
}
/* The `display_games` function in the `LobbyScreen` class is responsible for
updating the list of available games displayed on the lobby screen. */

void GameController::createGameRequested()
{
    qDebug() << "Creating game requested";
    client->onCreateRoomOk = [this](uint32_t gameId, uint8_t playersCount, const std::string &owner, const std::vector<std::string> &players) {
        qDebug() << "Game created successfully, id=" << (int)gameId;
        std::vector<QString> qplayers;
        for (const auto &p : players) qplayers.push_back(QString::fromStdString(p));
        emit createdGame(gameId);
        emit joinedGame(gameId, qplayers, true);
    };
    client->onError = [this](const std::string &reason) {
        qDebug() << "Error creating game:" << QString::fromStdString(reason);
    };
    client->createRoom();
}
