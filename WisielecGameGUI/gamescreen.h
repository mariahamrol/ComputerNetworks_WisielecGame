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
    void updateGameState(const QString &word, const std::vector<QString> &players, const std::vector<int> &lives, const std::vector<int> &points, const QString &guessedLetters, const QString &myGuessedLetters);
    void incrementMyMistakes();
    void updateMyMistakes(int mistakes);
    void resetKeyboard();
    void disableGuessedLetters(const QString &guessedLetters, const QString &myGuessedLetters);
    void disablePlayer();

signals:
    void letterClicked(QChar letter);
    void exitGame();

private:
    bool isEliminated = false;
    QLabel *wordLabel;
    QGridLayout *keyboardLayout;
    QMap<QChar, QPushButton*> letterButtons;
    QSet<QChar> clickedLetters;  // Lokalnie śledzone kliknięte litery
    QString currentWord;
    
    // Hangman widgets
    Hangman *myHangman;
    QLabel *myNickLabel;
    QLabel *myPointsLabel;
    QMap<QString, Hangman*> otherHangmans;
    QMap<QString, QLabel*> otherNickLabels;
    QMap<QString, QLabel*> otherPointsLabels;
    QHBoxLayout *otherHangmansLayout;
};

#endif // GAMESCREEN_H
