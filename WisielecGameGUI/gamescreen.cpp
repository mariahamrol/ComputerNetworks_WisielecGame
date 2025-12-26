#include "gamescreen.h"
#include "hangman.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QFontDatabase>

#define MAX_LIVES 8

GameScreen::GameScreen(QWidget *parent)
    : QWidget(parent)
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


    wordLabel = new QLabel("_ _ _ _ _");
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

    const QString letters = "AĄBCĆDEĘFGHIJKLŁMNŃOÓPQRSŚTUVWXYZŻŹ";
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
    setLayout(mainLayout);
}

void GameScreen::setHiddenWord(const QString &word) {
    currentWord = word;
    wordLabel->setText(word);
    resetKeyboard();  // Nowe słowo = reset klawiatury
    qDebug() << "Hidden word set to:" << word;
}

void GameScreen::setPlayers(const std::vector<QString> &players, const QString &myNick) {
    // TODO: Implement player list display
    qDebug() << "Players:" << players.size() << "My nick:" << myNick;
}

void GameScreen::updateGameState(const QString &word, const std::vector<int> &lives) {
    // Sprawdź czy to nowe słowo (nowa tura)
    if (currentWord != word) {
        currentWord = word;
        resetKeyboard();  // Nowe słowo = reset klawiatury
        qDebug() << "New word detected - keyboard reset";
    }
    
    // Zaktualizuj wyświetlane słowo (od serwera)
    wordLabel->setText(word);
    qDebug() << "Game state updated - Word:" << word << "Lives:" << lives.size();
    
    // TODO: Update lives display for all players
}

void GameScreen::updateGameState(const QString &word, const std::vector<int> &lives, const QString &guessedLetters) {
    // Sprawdź czy to nowe słowo (nowa tura)
    if (currentWord != word) {
        currentWord = word;
        resetKeyboard();  // Nowe słowo = reset klawiatury
        qDebug() << "New word detected - keyboard reset";
    }
    
    // Zaktualizuj wyświetlane słowo (od serwera)
    wordLabel->setText(word);
    
    // Dezaktywuj zgadnięte litery
    disableGuessedLetters(guessedLetters);
    
    qDebug() << "Game state updated - Word:" << word << "Lives:" << lives.size() << "Guessed:" << guessedLetters;
}

void GameScreen::incrementMyMistakes() {
    // TODO: Implement mistake increment logic
    qDebug() << "My mistakes incremented";
}

void GameScreen::resetKeyboard() {
    // Włącz wszystkie przyciski
    for (auto btn : letterButtons) {
        btn->setEnabled(true);
    }
    qDebug() << "Keyboard reset - all letters enabled";
}

void GameScreen::disableGuessedLetters(const QString &guessedLetters) {
    // Dezaktywuj litery które zostały już zgadnięte
    for (QChar letter : guessedLetters) {
        if (letterButtons.contains(letter)) {
            letterButtons[letter]->setEnabled(false);
        }
    }
}

