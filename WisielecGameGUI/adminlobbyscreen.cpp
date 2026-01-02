#include "adminlobbyscreen.h"
#include <QHBoxLayout>

AdminLobbyScreen::AdminLobbyScreen(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *main = new QVBoxLayout(this);
    header = new QLabel("Admin Lobby: aktywne gry");
    refreshButton = new QPushButton("Odśwież");
    connect(refreshButton, &QPushButton::clicked, this, [this](){ emit refreshRequested(); });
    main->addWidget(header);
    main->addWidget(refreshButton);

    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    container = new QWidget(this);
    layout = new QVBoxLayout(container);
    layout->setAlignment(Qt::AlignTop);
    scrollArea->setWidget(container);
    main->addWidget(scrollArea);
    setLayout(main);
}

void AdminLobbyScreen::displayGames(const std::vector<std::pair<int,int>> &games) {
    // clear
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    int totalPlayers = 0;
    for (auto &g : games) {
        totalPlayers += g.second;
        QWidget *row = new QWidget(this);
        QHBoxLayout *hl = new QHBoxLayout(row);
        QLabel *label = new QLabel(QString("Gra %1 (%2 graczy)").arg(g.first).arg(g.second));
        QPushButton *viewBtn = new QPushButton("Podgląd");
        QPushButton *killBtn = new QPushButton("Zakończ");
        hl->addWidget(label);
        hl->addStretch();
        hl->addWidget(viewBtn);
        hl->addWidget(killBtn);
        layout->addWidget(row);
        connect(viewBtn, &QPushButton::clicked, this, [this, g](){ emit viewRequested(g.first); });
        connect(killBtn, &QPushButton::clicked, this, [this, g](){ emit terminateRequested(g.first); });
    }
    layout->addStretch();
    header->setText(QString("Admin Lobby: aktywne gry | Razem gier: %1, razem graczy: %2").arg((int)games.size()).arg(totalPlayers));
}
