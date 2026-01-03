#ifndef ADMINLOBBYSCREEN_H
#define ADMINLOBBYSCREEN_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QMap>
#include <vector>
#include "../include/messages.h"

class AdminLobbyScreen : public QWidget {
    Q_OBJECT
public:
    explicit AdminLobbyScreen(QWidget *parent = nullptr);
    void displayGames(const MsgAdminGamesList &games);
    void displayUsers(const MsgAdminUsersList &users);
    void displayGameDetails(const MsgAdminGameDetails &details);
signals:
    void terminateRequested(int gameId);
    void viewRequested(int gameId);
private:
    QTabWidget *tabWidget;
    
    // Games tab
    QWidget *gamesTab;
    QScrollArea *gamesScrollArea;
    QWidget *gamesContainer;
    QVBoxLayout *gamesLayout;
    QLabel *gamesHeader;
    
    // Users tab
    QWidget *usersTab;
    QScrollArea *usersScrollArea;
    QWidget *usersContainer;
    QVBoxLayout *usersLayout;
    QLabel *usersHeader;
    
    // Store expanded game details widgets
    QMap<int, QWidget*> gameDetailsWidgets;
    
    QFont appFont;
};

#endif // ADMINLOBBYSCREEN_H
