#include "adminlobbyscreen.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QFontDatabase>
#include <QDebug>
#include <QFrame>
#include <vector>
#include "gamecontroller.h"

AdminLobbyScreen::AdminLobbyScreen(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *main = new QVBoxLayout(this);
    
    // Create tab widget
    tabWidget = new QTabWidget(this);
    
    // === GAMES TAB ===
    gamesTab = new QWidget();
    QVBoxLayout *gamesMainLayout = new QVBoxLayout(gamesTab);
    gamesHeader = new QLabel("Gry: 0");
    gamesHeader->setStyleSheet("font-size: 18px; font-weight: bold;");
    gamesMainLayout->addWidget(gamesHeader);
    

    gamesScrollArea = new QScrollArea();
    gamesScrollArea->setWidgetResizable(true);
    gamesContainer = new QWidget();
    gamesLayout = new QVBoxLayout(gamesContainer);
    gamesLayout->setAlignment(Qt::AlignTop);
    gamesScrollArea->setWidget(gamesContainer);
    gamesMainLayout->addWidget(gamesScrollArea);
    
    // === USERS TAB ===
    usersTab = new QWidget();
    QVBoxLayout *usersMainLayout = new QVBoxLayout(usersTab);
    usersHeader = new QLabel("Użytkownicy: 0");
    usersHeader->setStyleSheet("font-size: 18px; font-weight: bold;");
    usersMainLayout->addWidget(usersHeader);
    

    usersScrollArea = new QScrollArea();
    usersScrollArea->setWidgetResizable(true);
    usersContainer = new QWidget();
    usersLayout = new QVBoxLayout(usersContainer);
    usersLayout->setAlignment(Qt::AlignTop);
    usersScrollArea->setWidget(usersContainer);
    usersMainLayout->addWidget(usersScrollArea);
    
    // Add tabs
    tabWidget->addTab(gamesTab, "Gry");
    tabWidget->addTab(usersTab, "Użytkownicy");
    
    main->addWidget(tabWidget);
    setLayout(main);
}

void AdminLobbyScreen::displayGames(const MsgAdminGamesList &games) {
    // Clear existing
    QLayoutItem *item;
    while ((item = gamesLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    gameDetailsWidgets.clear();
    
    int totalPlayers = 0;
    for (uint32_t i = 0; i < games.games_count; ++i) {
        const AdminGameInfo &g = games.games[i];
        totalPlayers += g.players_count;
        
        QWidget *gameWidget = new QWidget();
        QVBoxLayout *gameVLayout = new QVBoxLayout(gameWidget);
        gameVLayout->setContentsMargins(0, 0, 0, 0);
        
        // Main game row
        QWidget *row = new QWidget();
        QHBoxLayout *hl = new QHBoxLayout(row);

        QString status = g.is_active ? "AKTYWNA" : "POKÓJ";
        QLabel *label = new QLabel(QString("Gra %1 | Graczy: %2 | Status: %3 | Owner: %4")
            .arg(g.game_id).arg(g.players_count).arg(status).arg(QString::fromUtf8(g.owner)));
        label->setFont(appFont);
        label->setStyleSheet("QLabel { font-size: 16px; }");

        QPushButton *viewBtn = new QPushButton("Szczegóły ▼");
        viewBtn->setStyleSheet("QPushButton {background-color: lightgreen; padding: 5px;}");
        viewBtn->setFont(appFont);

        QPushButton *killBtn = new QPushButton("Zakończ");
        killBtn->setStyleSheet("QPushButton {background-color: lightcoral; padding: 5px;}");
        killBtn->setFont(appFont);

        hl->addWidget(label);
        hl->addStretch();
        hl->addWidget(viewBtn);
        hl->addWidget(killBtn);
        
        gameVLayout->addWidget(row);
        
        // Details widget (hidden initially)
        QWidget *detailsWidget = new QWidget();
        detailsWidget->setVisible(false);
        QVBoxLayout *detailsLayout = new QVBoxLayout(detailsWidget);
        detailsWidget->setStyleSheet("background-color: #f0f0f0; border: 1px solid #ccc; padding: 10px;");
        
        QLabel *detailsLabel = new QLabel("Ładowanie szczegółów...");
        detailsLabel->setStyleSheet("font-size: 14px;");
        detailsLayout->addWidget(detailsLabel);
        
        gameDetailsWidgets[g.game_id] = detailsWidget;
        gameVLayout->addWidget(detailsWidget);
        
        gamesLayout->addWidget(gameWidget);
        
        // Connect buttons
        connect(viewBtn, &QPushButton::clicked, this, [this, g, viewBtn, detailsWidget](){
            if (detailsWidget->isVisible()) {
                detailsWidget->setVisible(false);
                viewBtn->setText("Szczegóły ▼");
            } else {
                detailsWidget->setVisible(true);
                viewBtn->setText("Szczegóły ▲");
                emit viewRequested(g.game_id);
            }
        });
        connect(killBtn, &QPushButton::clicked, this, [this, g](){ emit terminateRequested(g.game_id); });
    }

    gamesLayout->addStretch();
    gamesHeader->setText(QString("Gry: %1 | Razem graczy: %2").arg(games.games_count).arg(totalPlayers));
}

void AdminLobbyScreen::displayUsers(const MsgAdminUsersList &users) {
    // Clear existing
    QLayoutItem *item;
    while ((item = usersLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    const char* states[] = {"CONNECTED", "LOGGING_IN", "WAIT_ADMIN_PWD", "LOBBY", "IN_ROOM", "IN_GAME", "ADMIN"};
    
    for (uint32_t i = 0; i < users.users_count; ++i) {
        const AdminUserInfo &u = users.users[i];
        
        QWidget *row = new QWidget();
        QHBoxLayout *hl = new QHBoxLayout(row);

        QString infoText = QString("Nick: %1 | Stan: %2 | Gra: %3")
            .arg(QString::fromUtf8(u.nick))
            .arg(u.state < 7 ? states[u.state] : "UNKNOWN")
            .arg(u.game_id == -1 ? "brak" : QString::number(u.game_id));
            
        // Add lives and points only for players in games
        if (u.game_id != -1) {
            infoText += QString(" | Życia: %1 | Punkty: %2").arg(u.lives).arg(u.points);
        }
        
        QLabel *label = new QLabel(infoText);
        label->setFont(appFont);
        label->setStyleSheet("QLabel { font-size: 14px; padding: 5px; }");

        hl->addWidget(label);
        hl->addStretch();
        
        usersLayout->addWidget(row);
    }

    usersLayout->addStretch();
    usersHeader->setText(QString("Użytkownicy: %1").arg(users.users_count));
}

void AdminLobbyScreen::displayGameDetails(const MsgAdminGameDetails &details) {
    QWidget *detailsWidget = gameDetailsWidgets.value(details.game_id, nullptr);
    if (!detailsWidget) return;
    
    // Clear existing layout
    QLayout *oldLayout = detailsWidget->layout();
    if (oldLayout) {
        QLayoutItem *item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete oldLayout;
    }
    
    QVBoxLayout *layout = new QVBoxLayout(detailsWidget);
    layout->setContentsMargins(10, 10, 10, 10);
    
    QString status = details.is_active ? "AKTYWNA" : "POKÓJ";
    QLabel *statusLabel = new QLabel(QString("Status: %1").arg(status));
    statusLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    layout->addWidget(statusLabel);
    
    if (details.is_active) {
        QLabel *wordLabel = new QLabel(QString("🔑 SŁOWO (admin): %1").arg(QString::fromUtf8(details.word)));
        wordLabel->setStyleSheet("color: red; font-weight: bold; font-size: 16px;");
        layout->addWidget(wordLabel);
        
        QString guessedWord = "";
        for (int i = 0; i < details.word_length; ++i) {
            if (i > 0) guessedWord += " ";
            guessedWord += details.word_guessed[i] ? details.word_guessed[i] : '_';
        }
        QLabel *guessedLabel = new QLabel(QString("Stan odgadywania: %1").arg(guessedWord));
        layout->addWidget(guessedLabel);
        
        QString allGuessed = "";
        for (int i = 0; i < ALPHABET_SIZE && details.guessed_letters[i]; ++i) {
            if (i > 0) allGuessed += " ";
            allGuessed += details.guessed_letters[i];
        }
        QLabel *allGuessedLabel = new QLabel(QString("Odgadnięte litery (wszyscy): %1").arg(allGuessed));
        layout->addWidget(allGuessedLabel);
    }
    
    QLabel *playersLabel = new QLabel("\n--- Gracze ---");
    playersLabel->setStyleSheet("font-weight: bold;");
    layout->addWidget(playersLabel);
    
    for (int i = 0; i < details.player_count; ++i) {
        const AdminPlayerInfo &p = details.players[i];
        
        QString playerInfo = QString("%1. %2 | Życia: %3 | Punkty: %4 | Aktywny: %5 | Owner: %6")
            .arg(i + 1)
            .arg(QString::fromUtf8(p.nick))
            .arg(p.lives)
            .arg(p.points)
            .arg(p.is_active ? "TAK" : "NIE")
            .arg(p.is_owner ? "TAK" : "NIE");
            
        QLabel *playerLabel = new QLabel(playerInfo);
        playerLabel->setStyleSheet("font-size: 13px; padding-left: 10px;");
        layout->addWidget(playerLabel);
        
        if (details.is_active) {
            QString playerGuessed = "   Odgadnięte przez tego gracza: ";
            for (int j = 0; j < ALPHABET_SIZE && p.guessed_letters[j]; ++j) {
                if (j > 0) playerGuessed += " ";
                playerGuessed += p.guessed_letters[j];
            }
            QLabel *guessedLabel = new QLabel(playerGuessed);
            guessedLabel->setStyleSheet("font-size: 12px; padding-left: 20px; color: #666;");
            layout->addWidget(guessedLabel);
        }
    }
}
