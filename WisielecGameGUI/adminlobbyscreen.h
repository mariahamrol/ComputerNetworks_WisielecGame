#ifndef ADMINLOBBYSCREEN_H
#define ADMINLOBBYSCREEN_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <vector>

class AdminLobbyScreen : public QWidget {
    Q_OBJECT
public:
    explicit AdminLobbyScreen(QWidget *parent = nullptr);
    void displayGames(const std::vector<std::pair<int,int>> &games);
signals:
    void refreshRequested();
    void terminateRequested(int gameId);
    void viewRequested(int gameId);
private:
    QScrollArea *scrollArea;
    QWidget *container;
    QVBoxLayout *layout;
    QLabel *header;
    QPushButton *refreshButton;
};

#endif // ADMINLOBBYSCREEN_H
