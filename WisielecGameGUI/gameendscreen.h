#ifndef GAMEENDSCREEN_H
#define GAMEENDSCREEN_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QScrollArea>
#include <vector>

struct PlayerResult {
    QString nick;
    int points;
    bool wasActive; // false if player left or was eliminated
};

class GameEndScreen : public QWidget {
    Q_OBJECT
public:
    explicit GameEndScreen(QWidget *parent = nullptr);
    void displayResults(const std::vector<PlayerResult> &results);

signals:
    void returnToLobby();

private:
    QLabel *titleLabel;
    QScrollArea *scrollArea;
    QWidget *resultsContainer;
    QVBoxLayout *resultsLayout;
    QPushButton *returnButton;
    QFont appFont;
};

#endif // GAMEENDSCREEN_H
