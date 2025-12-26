#ifndef GAMESCREEN_H
#define GAMESCREEN_H

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMap>
#include <vector>

class Hangman;

class GameScreen : public QWidget {
    Q_OBJECT
public:
    explicit GameScreen(QWidget *parent = nullptr);

    void setHiddenWord(const QString &word);
    void setPlayers(const std::vector<QString> &players, const QString &myNick);
    void updateGameState(const QString &word, const std::vector<int> &lives);
    void updateGameState(const QString &word, const std::vector<int> &lives, const QString &guessedLetters);
    void incrementMyMistakes();
    void resetKeyboard();
    void disableGuessedLetters(const QString &guessedLetters);

signals:
    void letterClicked(QChar letter);

private:
    QLabel *wordLabel;
    QGridLayout *keyboardLayout;
    QMap<QChar, QPushButton*> letterButtons;
    QString currentWord;
};

#endif // GAMESCREEN_H
