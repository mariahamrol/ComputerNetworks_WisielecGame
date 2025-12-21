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

    connect(startButton, &QPushButton::clicked,
            this, &WaitingRoomScreen::startGameClicked);
}

void WaitingRoomScreen::setRoomState(
    int gameId,
    const std::vector<QString> &players,
    bool isHost)
{
    gameIdLabel->setText(QString("Game ID: %1").arg(gameId));

    QLayoutItem *item;
    while ((item = playersLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    for (const auto &nick : players) {
        playersLayout->addWidget(new QLabel(nick));
    }

    startButton->setEnabled(isHost);
    if(isHost){
        startButton->setStyleSheet("QPushButton {background-color: lightgreen}");
    }

    connect(startButton, &QPushButton::clicked, this, [=]() {
        emit startGame(gameId);
    });
}
