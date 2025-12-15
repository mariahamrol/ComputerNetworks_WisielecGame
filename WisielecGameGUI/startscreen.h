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

signals:
    void startClicked(const char login[30]);

private:
    QPushButton *start_button;
    QLabel *welcome_label;
    QLineEdit *login_enter;
    QLabel *picture;        // for logo/image
    char login[30];
};

#endif // STARTSCREEN_H
