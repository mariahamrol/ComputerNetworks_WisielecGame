#ifndef LOBBYSCREEN_H
#define LOBBYSCREEN_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QScrollArea>
#include <vector>

class LobbyScreen : public QWidget
{
    Q_OBJECT

public:
    explicit LobbyScreen(QWidget *parent = nullptr);

    void set_login(const QString &login);
    void display_games(const std::vector<int> &gameIds);

signals:
    void joinGame(int gameId);

private:
    QLabel *welcomeLabel;

    QScrollArea *scrollArea;
    QWidget *gamesWidget;
    QVBoxLayout *gamesLayout;

    QFont appFont;
    QString *login;
};

#endif // LOBBYSCREEN_H
