#include "gameendscreen.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QDebug>
#include <algorithm>

GameEndScreen::GameEndScreen(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // Title
    titleLabel = new QLabel(" KONIEC GRY ");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 32px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // Results header
    QLabel *rankingLabel = new QLabel(" RANKING GRACZY ");
    rankingLabel->setAlignment(Qt::AlignCenter);
    rankingLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #e74c3c; margin-bottom: 10px;");
    mainLayout->addWidget(rankingLabel);

    // Scroll area for results
    scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: 2px solid #bdc3c7; border-radius: 10px; background-color: #ecf0f1; }");
    
    resultsContainer = new QWidget();
    resultsLayout = new QVBoxLayout(resultsContainer);
    resultsLayout->setAlignment(Qt::AlignTop);
    resultsLayout->setSpacing(10);
    resultsLayout->setContentsMargins(15, 15, 15, 15);
    
    scrollArea->setWidget(resultsContainer);
    mainLayout->addWidget(scrollArea);

    // Return button
    returnButton = new QPushButton("Powrót do Lobby");
    returnButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #3498db;"
        "   color: white;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "   padding: 15px;"
        "   border-radius: 10px;"
        "   border: none;"
        "}"
        "QPushButton:hover {"
        "   background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #21618c;"
        "}"
    );
    connect(returnButton, &QPushButton::clicked, this, &GameEndScreen::returnToLobby);
    mainLayout->addWidget(returnButton);

    setLayout(mainLayout);
}

void GameEndScreen::displayResults(const std::vector<PlayerResult> &results) {
    // Clear existing results
    QLayoutItem *item;
    while ((item = resultsLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // Sort results by points (descending)
    std::vector<PlayerResult> sortedResults = results;
    std::sort(sortedResults.begin(), sortedResults.end(), 
        [](const PlayerResult &a, const PlayerResult &b) {
            return a.points > b.points;
        });

    // Display results
    for (size_t i = 0; i < sortedResults.size(); ++i) {
        const PlayerResult &player = sortedResults[i];
        
        QWidget *playerWidget = new QWidget();
        QHBoxLayout *playerLayout = new QHBoxLayout(playerWidget);
        playerLayout->setContentsMargins(15, 10, 15, 10);
        
        QString bgColor;
        
        playerWidget->setStyleSheet(QString(
            "QWidget {"
            "   background-color: %1;"
            "   border: 2px solid #95a5a6;"
            "   border-radius: 8px;"
            "   padding: 5px;"
            "}"
        ).arg(bgColor));
        
        // Player name
        QLabel *nameLabel = new QLabel(player.nick);
        nameLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50;");
        playerLayout->addWidget(nameLabel);
        
        playerLayout->addStretch();
        
        // Points
        QLabel *pointsLabel = new QLabel(QString("%1 pkt").arg(player.points));
        pointsLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #27ae60; min-width: 100px;");
        pointsLabel->setAlignment(Qt::AlignRight);
        playerLayout->addWidget(pointsLabel);
        
        resultsLayout->addWidget(playerWidget);
    }
    
    resultsLayout->addStretch();
}
