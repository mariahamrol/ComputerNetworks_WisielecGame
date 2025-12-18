#include "gamescreen.h"
#include "hangman.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QFontDatabase>

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
            emit letterClicked(letter);
            qDebug() << "Letter clicked:" << letter;
        });

        keyboardLayout->addWidget(btn, row, col);

        col++;
        if (col == 7) {
            col = 0;
            row++;
        }
    }

    middleLayout->addWidget(keyboardWidget);

    Hangman *myHangman = new Hangman();
    myHangman->setMinimumSize(300,300);
    myHangman->setMistakes(8);
    middleLayout->addWidget(myHangman);

    QHBoxLayout *playersLayout = new QHBoxLayout();
    mainLayout->addLayout(playersLayout);
    for(int i=0;i<5;i++){
        Hangman *otherHangman = new Hangman();
        otherHangman->setMinimumSize(150,150);
        otherHangman->setMistakes(i);
        playersLayout->addWidget(otherHangman);
    }
}
