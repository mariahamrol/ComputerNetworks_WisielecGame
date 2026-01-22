#include "adminloginscreen.h"
#include <QVBoxLayout>
#include <QFontDatabase>
#include <QPixmap>
#include <QDebug>
#include <QMessageBox>
#include <QCoreApplication>

AdminLoginScreen::AdminLoginScreen(QWidget *parent) : QWidget(parent) {
    QFont appFont;
    int fontId = QFontDatabase::addApplicationFont(QCoreApplication::applicationDirPath() + "/../assets/fonts/Orbitron-VariableFont_wght.ttf");
    if (fontId != -1) {
        QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
        appFont = QFont(family, 20, QFont::Bold);
    } else {
        qDebug() << "Failed to load font!";
        appFont = QFont("Arial", 18, QFont::Bold);
    }


    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addStretch(2);

    //Password label
    infoLabel = new QLabel("Podaj hasło admina:");
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setFont(appFont);
    layout->addWidget(infoLabel, 0, Qt::AlignHCenter);

    //Password input
    passwordEdit = new QLineEdit();
    passwordEdit->setPlaceholderText("Enter password");
    passwordEdit->setFont(appFont);
    passwordEdit->setMaximumWidth(300);
    passwordEdit->setAlignment(Qt::AlignHCenter);
    passwordEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(passwordEdit, 0, Qt::AlignHCenter);

    //Submit button
    submitButton = new QPushButton("Zaloguj");
    submitButton->setFont(appFont);
    submitButton->setMaximumWidth(300);
    submitButton->setEnabled(false);
    layout->addWidget(submitButton, 0, Qt::AlignHCenter);

    layout->addStretch(3);
    setLayout(layout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(passwordEdit, &QLineEdit::textChanged, this, [=](const QString &text){
        submitButton->setEnabled(!text.trimmed().isEmpty());
    });
    connect(submitButton, &QPushButton::clicked, this, [=](){
        emit submitPassword(passwordEdit->text());
    });
}

void AdminLoginScreen::showError(const QString &msg) {
    QMessageBox box(QMessageBox::Warning, "Admin login failed        ", msg, QMessageBox::Ok, this);
    box.setMinimumSize(400, 200);
    box.setStyleSheet(
        "QMessageBox {"
        "  background-color: #ffffff;"
        "} "
        "QLabel { color: black; font-size: 16px; } "
        "QPushButton { min-width: 80px; min-height: 30px; }"
    );
    box.exec();
    passwordEdit->clear();
    passwordEdit->setFocus();
}
