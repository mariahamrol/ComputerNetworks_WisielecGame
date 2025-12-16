#include "gamecontroller.h"
#include <QDebug>

GameController::GameController(QObject *parent)
    : QObject(parent) {}

void GameController::loginRequested(const QString &login)
{
    // MOCK – później: MSG_LOGIN_REQ
    if (login.isEmpty()) {
        emit loginRejected("Empty login");
        return;
    }

    emit loginAccepted();
    qDebug() << "Login accepted" << login;

    std::vector<int> games;
    for (int i = 1; i <= 10; ++i)
        games.push_back(i);

    emit lobbyStateUpdated(games);
}

void GameController::joinGameRequested(int gameId)
{
    // MOCK – później: MSG_JOIN_GAME_REQ
    qDebug() << "Joining game" << gameId;
    std::vector<QString> players = {
        "alice",
        "bob",
        "you"
    };

    bool isHost = true;

    emit joinedGame(gameId,players,isHost);
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
