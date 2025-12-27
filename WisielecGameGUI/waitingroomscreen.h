#ifndef WAITINGROOMSCREEN_H
#define WAITINGROOMSCREEN_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <vector>


class WaitingRoomScreen : public QWidget
{
    Q_OBJECT
public:
    explicit WaitingRoomScreen(QWidget *parent = nullptr);

    void setRoomState(
    int gameId,
    const std::vector<QString> &players,
    const QString &owner,
    bool isHost
    );

    void setRoomOwner(const QString &owner);

    signals:
        void startGameClicked();
        void leaveRoomClicked();
        void startGame(int gameId);
        void exitRoom();

    private:
        int currentGameId = -1;
        QLabel *gameIdLabel;
        QVBoxLayout *playersLayout;
        QPushButton *startButton;
        QLabel *ownerLabel;
        // QLabel *playersCountLabel;
};

#endif // WAITINGROOMSCREEN_H
