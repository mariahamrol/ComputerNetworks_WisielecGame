#ifndef WAITINGROOMSCREEN_H
#define WAITINGROOMSCREEN_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QScrollArea>
#include <vector>

class WaitingRoomScreen : public QWidget
{
    Q_OBJECT

public:
    explicit WaitingRoomScreen(QWidget *parent = nullptr);

    void set_login(const char *loginText);
    void set_games(const std::vector<int> &gameIds);

signals:
    void joinGame(int gameId);

private:
    QLabel *welcomeLabel;

    QScrollArea *scrollArea;
    QWidget *gamesWidget;
    QVBoxLayout *gamesLayout;

    QFont appFont;
    char login[30];
};

#endif // WAITINGROOMSCREEN_H
