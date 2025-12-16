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
        bool isHost
        );

    signals:
        void startGameClicked();
        void leaveRoomClicked();
        void startGame(int gameId);

    private:
        QLabel *gameIdLabel;
        QVBoxLayout *playersLayout;
        QPushButton *startButton;
};

#endif // WAITINGROOMSCREEN_H
