#ifndef GAMESCREEN_H
#define GAMESCREEN_H

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <vector>

class GameScreen : public QWidget {
    Q_OBJECT
public:
    explicit GameScreen(QWidget *parent = nullptr);

    void setHiddenWord(const QString &word);
    void setPlayers(const std::vector<QString> &players);

signals:
    void letterClicked(QChar letter);

private:
    QLabel *wordLabel;
    QGridLayout *keyboardLayout;
};

#endif // GAMESCREEN_H
