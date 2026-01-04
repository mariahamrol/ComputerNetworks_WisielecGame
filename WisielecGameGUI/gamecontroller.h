#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <QObject>
#include <vector>
#include <QVariant>
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
    void guessLetterRequested(QChar letter);
    void exitGameRequested();
    void exitRoomRequested();
    void reconnectToServer(const QString &serverIp);
    void connectToServerInitial(const QString &serverIp);
    // Admin slots
    void adminLoginRequested(const QString &password);
    void adminListGamesRequested();
    void adminListUsersRequested();
    void adminGameDetailsRequested(int gameId);
    void adminTerminateGameRequested(int gameId);
    // bool give_mistakes = true;

signals:
    void connectionError(); 
    void connectionSuccessful();
    void serverDisconnected();
    void loginAccepted(const QString &login);
    void loginRejected(const QString &reason);

    void lobbyStateUpdated(const std::vector<int> &games);
    void createdGame(int gameId);
    void joinedGame(int gameId, std::vector<QString> players, QString owner, bool isHost);
    void gameStateUpdated(int gameId, QString hiddenWord, std::vector<QString> players, std::vector<int> lives, std::vector<int> points, QString guessedLetters, QString myGuessedLetters);
    void gameStarted( int gameId, QString hiddenWord, std::vector<QString> players, QString myNick);
    void gameEnded(std::vector<QString> playerNames, std::vector<int> points, std::vector<bool> wasActive);
    void wrongLetterGuessed();
    void playerEliminated();
    void exitedGame();
    void exitedRoom();
    void roomClosed();
    void startGameFailed();
    // Admin signals
    void adminPasswordRequired();
    void adminLoginOk();
    void adminLoginFail();
    void adminGamesListUpdated(const MsgAdminGamesList &games);
    void adminUsersListUpdated(const MsgAdminUsersList &users);
    void adminGameDetailsUpdated(const MsgAdminGameDetails &details);
    void adminTerminateOk();
    void adminTerminateFail();
private:
    ClientConnection *client;
    QString myNickname;
    bool inGame = false;
    bool waitingForGameStart = false;
};
#endif // GAMECONTROLLER_H
