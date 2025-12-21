#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <QObject>
#include <vector>
#include "client/ClientConnection.h"

class GameController : public QObject {
    Q_OBJECT
public:
    explicit GameController(QObject *parent = nullptr);

public slots:
    void loginRequested(const QString &login);
    void joinGameRequested(int gameId);
    void startGameRequested(int gameId);
    void createGameRequested();
    // bool give_mistakes = true;

signals:
    void loginAccepted(const QString &login);
    void loginRejected(const QString &reason);

    void lobbyStateUpdated(const std::vector<int> &games);
    void createdGame(int gameId);
    void joinedGame(int gameId, std::vector<QString> players, bool isHost);
    void gameStarted( int gameId, QString hiddenWord, std::vector<QString> players);
private:
    ClientConnection *client;
};
#endif // GAMECONTROLLER_H
