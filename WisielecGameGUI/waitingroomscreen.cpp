#include "waitingroomscreen.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QFontDatabase>
#include <QDebug>
#include <vector>

WaitingRoomScreen::WaitingRoomScreen(QWidget *parent)
    : QWidget(parent)
{
    // -------- Load font --------
    int fontId = QFontDatabase::addApplicationFont(
        "C:/Users/marha/Documents/studia/term_5/sk2/ComputerNetworks_WisielecGame/"
        "WisielecGameGUI/assets/fonts/Orbitron-VariableFont_wght.ttf");

    if (fontId != -1) {
        QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
        appFont = QFont(family, 16, QFont::Bold);
    } else {
        qDebug() << "Font load failed, using default";
        appFont = QFont("Arial", 16, QFont::Bold);
    }

    //Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);

    // Welcome label checking if we got the login
    welcomeLabel = new QLabel("Welcome!");
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setFont(appFont);
    mainLayout->addWidget(welcomeLabel);

    //Scroll area withy all the games in the list
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);

    gamesWidget = new QWidget(this);
    gamesLayout = new QVBoxLayout(gamesWidget);
    gamesLayout->setAlignment(Qt::AlignTop);
    gamesLayout->setSpacing(10);

    scrollArea->setWidget(gamesWidget);
    mainLayout->addWidget(scrollArea);

    setLayout(mainLayout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void WaitingRoomScreen::set_login(const char *loginText)
{
    if (!loginText) return;

    strncpy(login, loginText, sizeof(login));
    login[sizeof(login) - 1] = '\0';

    welcomeLabel->setText(QString("Welcome to the game %1").arg(login));
}

void WaitingRoomScreen::set_games(const std::vector<int> &gameIds)
{
    QLayoutItem *item;
    while ((item = gamesLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    for (int gameId : gameIds) {
        QWidget *row = new QWidget(this);
        QHBoxLayout *rowLayout = new QHBoxLayout(row);

        QLabel *label = new QLabel(QString("Game ID: %1").arg(gameId));
        label->setFont(appFont);

        QPushButton *joinButton = new QPushButton("Join");
        joinButton->setFont(appFont);
        joinButton->setStyleSheet("QPushButton {background-color: lightgreen}");

        rowLayout->addWidget(label);
        rowLayout->addStretch();
        rowLayout->addWidget(joinButton);

        gamesLayout->addWidget(row);

        connect(joinButton, &QPushButton::clicked, this, [=]() {
            emit joinGame(gameId);
        });
    }

    gamesLayout->addStretch();
}
