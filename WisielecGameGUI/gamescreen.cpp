#include "gamescreen.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>

GameScreen::GameScreen(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);


    wordLabel = new QLabel("_ _ _ _ _");
    QFont wordFont;
    wordFont.setPointSize(28);
    wordFont.setBold(true);
    wordLabel->setFont(wordFont);
    wordLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(wordLabel);


    QHBoxLayout *middleLayout = new QHBoxLayout();
    mainLayout->addLayout(middleLayout);


    QVBoxLayout *hangmanLayout = new QVBoxLayout();
    QLabel *hangmanTitle = new QLabel("Players");
    hangmanLayout->addWidget(hangmanTitle);
    middleLayout->addLayout(hangmanLayout);

    // Keyboard
    QWidget *keyboardWidget = new QWidget(this);
    keyboardLayout = new QGridLayout(keyboardWidget);

    const QString letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int row = 0, col = 0;

    for (QChar letter : letters) {
        QPushButton *btn = new QPushButton(letter);
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
}
