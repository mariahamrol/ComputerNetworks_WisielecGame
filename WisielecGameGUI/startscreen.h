#ifndef STARTSCREEN_H
#define STARTSCREEN_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

class StartScreen : public QWidget
{
    Q_OBJECT

public:
    explicit StartScreen(QWidget *parent = nullptr);
    bool isConnected = false;
public slots:
    void showLoginError(const QString &msg);
    void showConnectionError(const QString &msg);
    void hideConnectionError();

signals:
    void startClicked(const QString login);
    void reconnectRequested();
    void closeApplicationRequested();

private:
    QPushButton *start_button;
    QLabel *welcome_label;
    QLineEdit *login_enter;
    QLabel *picture;        // for logo/image
    QString *login;
    
    // Connection error widgets
    QWidget *error_widget;
    QLabel *error_label;
    QPushButton *retry_button;
    QPushButton *close_button;
};

#endif // STARTSCREEN_H
