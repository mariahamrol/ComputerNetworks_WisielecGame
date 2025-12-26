#include "waitingroomscreen.h"
#include <QVBoxLayout>
#include <QFontDatabase>

WaitingRoomScreen::WaitingRoomScreen(QWidget *parent)
    : QWidget(parent)
{
    QFont appFont;
    int fontId = QFontDatabase::addApplicationFont("./assets/fonts/Orbitron-VariableFont_wght.ttf");
    if (fontId != -1) {
        QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
        appFont = QFont(family);
    } else {
        qDebug() << "Failed to load font!";
        appFont = QFont("Arial");
    }

    QVBoxLayout *main = new QVBoxLayout(this);
    gameIdLabel = new QLabel("Game ID: ");
    gameIdLabel->setAlignment(Qt::AlignCenter);
    QFont gameIdFont = appFont;
    gameIdFont.setBold(true);
    gameIdFont.setPointSize(35);
    gameIdLabel->setFont(gameIdFont);
    main->addWidget(gameIdLabel);

    QWidget *playersWidget = new QWidget(this);
    QFont playersFont = appFont;
    playersFont.setBold(true);
    playersFont.setPointSize(25);
    playersWidget->setFont(playersFont);
    playersLayout = new QVBoxLayout(playersWidget);
    playersLayout->setAlignment(Qt::AlignCenter);
    main->addWidget(playersWidget);

    startButton = new QPushButton("Start Game");
    startButton->setEnabled(false);
    QFont buttonFont = appFont;
    buttonFont.setBold(true);
    buttonFont.setPointSize(25);
    startButton->setFont(buttonFont);
    main->addWidget(startButton);

    ownerLabel = new QLabel(this);
    ownerLabel->setAlignment(Qt::AlignCenter);
    ownerLabel->setFont(playersFont);
    main->addWidget(ownerLabel);

    // playersCountLabel = new QLabel(this);
    // playersCountLabel->setAlignment(Qt::AlignCenter);
    // playersCountLabel->setFont(playersFont);
    // main->addWidget(playersCountLabel);

    connect(startButton, &QPushButton::clicked, this, [this]() {
    if (currentGameId != -1) {
        emit startGame(currentGameId);
    }
});

}

void WaitingRoomScreen::setRoomState(
    int gameId,
    const std::vector<QString> &players,
    const QString &owner,
    bool isHost)
{
    gameIdLabel->setText(QString("Game ID: %1").arg(gameId));
    currentGameId = gameId;
    ownerLabel->setText(QString("Host: %1").arg(owner));
    // playersCountLabel->setText(
    //     QString("Players: %1").arg(players.size())
    // );

    QLayoutItem *item;
    while ((item = playersLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    for (const auto &nick : players) {
        QLabel *lbl = new QLabel(nick);
        playersLayout->addWidget(lbl);
    }

    startButton->setEnabled(isHost);
    startButton->setVisible(isHost);

    if (isHost) {
        startButton->setStyleSheet(
            "QPushButton { background-color: lightgreen; }"
        );
    }
}

