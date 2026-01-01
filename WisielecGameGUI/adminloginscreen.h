#ifndef ADMINLOGINSCREEN_H
#define ADMINLOGINSCREEN_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class AdminLoginScreen : public QWidget {
    Q_OBJECT
public:
    explicit AdminLoginScreen(QWidget *parent = nullptr);
public slots:
    void showError(const QString &msg);
signals:
    void submitPassword(const QString &password);
private:
    QLabel *infoLabel;
    QLineEdit *passwordEdit;
    QPushButton *submitButton;
};

#endif // ADMINLOGINSCREEN_H
