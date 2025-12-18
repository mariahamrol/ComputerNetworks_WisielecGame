#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <QObject>
#include <vector>

class GameController : public QObject {
    Q_OBJECT
public:
    explicit GameController(QObject *parent = nullptr);

public slots:
    void loginRequested(const QString &login);
    void joinGameRequested(int gameId);
    void startGameRequested(int gameId);
    // bool give_mistakes = true;

signals:
    void loginAccepted();
    void loginRejected(const QString &reason);

    void lobbyStateUpdated(const std::vector<int> &games);
    void joinedGame(int gameId, std::vector<QString> players, bool isHost);
    void gameStarted( int gameId, QString hiddenWord, std::vector<QString> players);
};
#endif // GAMECONTROLLER_H
