#include "gamescreen.h"
#include "hangman.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QFontDatabase>
#include <QSet>

#define MAX_LIVES 10

GameScreen::GameScreen(QWidget *parent)
    : QWidget(parent), isEliminated(false)
{

    QFont appFont;
    int fontId = QFontDatabase::addApplicationFont("C:/Users/marha/Documents/studia/term_5/sk2/ComputerNetworks_WisielecGame/WisielecGameGUI/assets/fonts/Orbitron-VariableFont_wght.ttf");
    if (fontId != -1) {
        QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
        appFont = QFont(family);
    } else {
        qDebug() << "Failed to load font!";
        appFont = QFont("Arial");
    }

    QVBoxLayout *mainLayout = new QVBoxLayout(this);


    wordLabel = new QLabel();
    QFont wordFont = appFont;
    wordFont.setPointSize(28);
    wordFont.setBold(true);
    wordLabel->setFont(wordFont);
    wordLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(wordLabel);


    QHBoxLayout *middleLayout = new QHBoxLayout();
    mainLayout->addLayout(middleLayout);

    // Keyboard
    QWidget *keyboardWidget = new QWidget(this);
    keyboardLayout = new QGridLayout(keyboardWidget);

    const QString letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int row = 0, col = 0;

    for (QChar letter : letters) {
        QPushButton *btn = new QPushButton(letter);
        QFont buttonFont = appFont;
        buttonFont.setBold(true);
        buttonFont.setPointSize(10);
        btn->setFont(buttonFont);
        btn->setFixedSize(40, 40);

        connect(btn, &QPushButton::clicked, this, [=]() {
            btn->setEnabled(false);
            clickedLetters.insert(letter);  // Zapamiętaj klikniętą literę
            emit letterClicked(letter);
            qDebug() << "Letter clicked:" << letter;
        });

        keyboardLayout->addWidget(btn, row, col);
        letterButtons[letter] = btn;  // Zachowaj referencję do przycisku

        col++;
        if (col == 7) {
            col = 0;
            row++;
        }
    }

    middleLayout->addWidget(keyboardWidget);

    // Mój wisielec (większy, obok klawiatury)
    QVBoxLayout *myHangmanLayout = new QVBoxLayout();
    myHangmanLayout->setAlignment(Qt::AlignCenter);
    middleLayout->addLayout(myHangmanLayout);
    
    myHangman = new Hangman(this);
    myHangman->setFixedSize(200, 200);
    myHangman->setMistakes(0);  // Początek z 0 błędami
    myHangmanLayout->addWidget(myHangman, 0, Qt::AlignCenter);
    
    // Nick i punkty w poziomym layoutcie
    QHBoxLayout *myInfoLayout = new QHBoxLayout();
    myInfoLayout->setAlignment(Qt::AlignCenter);
    
    myNickLabel = new QLabel("My Nick", this);
    myNickLabel->setAlignment(Qt::AlignCenter);
    QFont nickFont = appFont;
    nickFont.setPointSize(12);
    nickFont.setBold(true);
    myNickLabel->setFont(nickFont);
    myInfoLayout->addWidget(myNickLabel);
    
    myPointsLabel = new QLabel("0 pts", this);
    myPointsLabel->setAlignment(Qt::AlignCenter);
    QFont pointsFont = appFont;
    pointsFont.setPointSize(10);
    myPointsLabel->setFont(pointsFont);
    myPointsLabel->setStyleSheet("color: #4CAF50; margin-left: 10px;");
    myInfoLayout->addWidget(myPointsLabel);
    
    myHangmanLayout->addLayout(myInfoLayout);

    // Wisielce innych graczy (mniejsze, pod klawiaturą)
    otherHangmansLayout = new QHBoxLayout();
    otherHangmansLayout->setAlignment(Qt::AlignCenter);
    mainLayout->addLayout(otherHangmansLayout);
    
    // Przycisk Exit na dole
    QPushButton *exitButton = new QPushButton("Exit Game", this);
    QFont exitFont = appFont;
    exitFont.setPointSize(14);
    exitFont.setBold(true);
    exitButton->setFont(exitFont);
    exitButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; padding: 10px; }");
    connect(exitButton, &QPushButton::clicked, this, &GameScreen::exitGame);
    mainLayout->addWidget(exitButton);

    setLayout(mainLayout);
}

void GameScreen::setHiddenWord(const QString &word) {
    currentWord = word;
    wordLabel->setText(word);
    resetKeyboard();  // Nowe słowo = reset klawiatury
    qDebug() << "Hidden word set to:" << word;
}

void GameScreen::setPlayers(const std::vector<QString> &players, const QString &myNick) {
    qDebug() << "Setting up players. Total:" << players.size() << "My nick:" << myNick;
    
    // Ustaw swój nick
    myNickLabel->setText(myNick);
    
    // Wyczyść poprzednie wisielce innych graczy
    for (auto hangman : otherHangmans.values()) {
        delete hangman;
    }
    for (auto label : otherNickLabels.values()) {
        delete label;
    }
    otherHangmans.clear();
    otherNickLabels.clear();
    
    // Utwórz wisielce dla innych graczy
    QFont nickFont;
    nickFont.setPointSize(10);
    nickFont.setBold(true);
    
    for (const QString &playerNick : players) {
        if (playerNick == myNick) {
            continue; // Pomijamy siebie
        }
        
        // Utwórz pionowy layout dla każdego gracza (wisielec + nick)
        QVBoxLayout *playerLayout = new QVBoxLayout();
        playerLayout->setAlignment(Qt::AlignCenter);
        playerLayout->setSpacing(5);
        
        // Wisielec
        Hangman *hangman = new Hangman(this);
        hangman->setFixedSize(200, 200);
        hangman->setMistakes(0);  // Początek z 0 błędami
        playerLayout->addWidget(hangman, 0, Qt::AlignCenter);
        otherHangmans[playerNick] = hangman;
        
        // Nick pod wisielcem
        QLabel *nickLabel = new QLabel(playerNick, this);
        nickLabel->setAlignment(Qt::AlignCenter);
        nickLabel->setFont(nickFont);
        playerLayout->addWidget(nickLabel, 0, Qt::AlignCenter);
        otherNickLabels[playerNick] = nickLabel;
        
        otherHangmansLayout->addLayout(playerLayout);
    }
    
    qDebug() << "Created" << otherHangmans.size() << "other player hangmans";
}

void GameScreen::updateGameState(const QString &word, const std::vector<QString> &players, const std::vector<int> &lives, const std::vector<int> &points, const QString &guessedLetters, const QString &myGuessedLetters) {
    // Sprawdź czy to nowe słowo (nowa tura)
    bool isNewWord = (currentWord != word);
    if (isNewWord) {
        currentWord = word;
        resetKeyboard();  // Nowe słowo = reset klawiatury
        qDebug() << "New word detected - keyboard reset";
    }
    
    // Zaktualizuj wyświetlane słowo (od serwera)
    wordLabel->setText(word);
    
    // Zawsze dezaktywuj zgadnięte litery z serwera + lokalnie kliknięte
    disableGuessedLetters(guessedLetters, myGuessedLetters);
    
    // Zaktualizuj wisielce i punkty na podstawie żyć
    for (size_t i = 0; i < players.size() && i < lives.size(); ++i) {
        const QString &playerNick = players[i];
        int playerLives = lives[i];
        int playerPoints = (i < points.size()) ? points[i] : 0;
        int mistakes = MAX_LIVES - playerLives;
        
        if (playerNick == myNickLabel->text()) {
            // To ja - zaktualizuj mój wisielec i punkty
            myHangman->setMistakes(mistakes);
            myPointsLabel->setText(QString("%1 pts").arg(playerPoints));
            qDebug() << "Updated my hangman:" << mistakes << "mistakes," << playerPoints << "points";
        } else if (otherHangmans.contains(playerNick)) {
            // Inny gracz - zaktualizuj jego wisielca
            otherHangmans[playerNick]->setMistakes(mistakes);
            qDebug() << "Updated" << playerNick << "hangman:" << mistakes << "mistakes";
        }
    }
    
    qDebug() << "Game state updated - Word:" << word << "Lives:" << lives.size() << "Global guessed:" << guessedLetters << "My guessed:" << myGuessedLetters;
}

void GameScreen::incrementMyMistakes() {
    // TODO: Implement mistake increment logic
    qDebug() << "My mistakes incremented";
}

void GameScreen::updateMyMistakes(int mistakes) {
    myHangman->setMistakes(mistakes);
    qDebug() << "[GameScreen] Updated my hangman to" << mistakes << "mistakes";
}

void GameScreen::resetKeyboard() {
    // Jeśli gracz jest wyeliminowany, nie włączaj przycisków
    if (isEliminated) {
        qDebug() << "Keyboard reset skipped - player is eliminated";
        return;
    }
    
    // Wyczyść lokalnie zapamiętane kliknięte litery
    clickedLetters.clear();
    
    // Włącz wszystkie przyciski
    for (auto btn : letterButtons) {
        btn->setEnabled(true);
    }
    qDebug() << "Keyboard reset - all letters enabled";
}

void GameScreen::disableGuessedLetters(const QString &guessedLetters, const QString &myGuessedLetters) {
    qDebug() << "[GameScreen] disableGuessedLetters called";
    qDebug() << "[GameScreen] Global guessed:" << guessedLetters;
    qDebug() << "[GameScreen] My guessed:" << myGuessedLetters;
    
    // Dezaktywuj litery które zostały poprawnie zgadnięte (dla wszystkich graczy)
    for (QChar letter : guessedLetters) {
        QChar upperLetter = letter.toUpper();
        if (letterButtons.contains(upperLetter)) {
            letterButtons[upperLetter]->setEnabled(false);
            qDebug() << "[GameScreen] Disabled button for all:" << upperLetter;
        }
    }
    
    // Dezaktywuj litery które zgadł ten gracz (niezależnie czy poprawne czy nie)
    for (QChar letter : myGuessedLetters) {
        QChar upperLetter = letter.toUpper();
        if (letterButtons.contains(upperLetter)) {
            letterButtons[upperLetter]->setEnabled(false);
            qDebug() << "[GameScreen] Disabled button for me:" << upperLetter;
        }
    }
    
    // Dezaktywuj wszystkie lokalnie kliknięte litery (nawet jeśli serwer jeszcze ich nie potwierdził)
    for (QChar letter : clickedLetters) {
        QChar upperLetter = letter.toUpper();
        if (letterButtons.contains(upperLetter)) {
            letterButtons[upperLetter]->setEnabled(false);
            qDebug() << "[GameScreen] Disabled button (locally clicked):" << upperLetter;
        }
    }
}

void GameScreen::disablePlayer() {
    isEliminated = true;
    for (auto btn : letterButtons) {
        btn->setEnabled(false);
    }
}

