#include "startscreen.h"
#include <QVBoxLayout>
#include <QFontDatabase>
#include <QPixmap>
#include <QDebug>

StartScreen::StartScreen(QWidget *parent)
    : QWidget(parent)
{
    QFont appFont;
    int fontId = QFontDatabase::addApplicationFont("C:/Users/marha/Documents/studia/term_5/sk2/ComputerNetworks_WisielecGame/WisielecGameGUI/assets/fonts/Orbitron-VariableFont_wght.ttf");
    if (fontId != -1) {
        QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
        appFont = QFont(family, 20, QFont::Bold);
    } else {
        qDebug() << "Failed to load font!";
        appFont = QFont("Arial", 18, QFont::Bold);
    }

    QVBoxLayout *layout = new QVBoxLayout(this);

    //Welcome to the game label
    welcome_label = new QLabel("Welcome to Wisielec Game");
    welcome_label->setAlignment(Qt::AlignCenter);
    welcome_label->setFont(appFont);
    layout->addWidget(welcome_label,0,Qt::AlignHCenter);

    //Picture just so there is something happening here
    picture = new QLabel();
    QPixmap pixmap("C:/Users/marha/Documents/studia/term_5/sk2/ComputerNetworks_WisielecGame/WisielecGameGUI/assets/icons/wisielec.png");
    if (!pixmap.isNull()) {
        picture->setPixmap(pixmap.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    // picture->setAlignment(Qt::AlignCenter);
    layout->addWidget(picture,0,Qt::AlignHCenter);

    //Place to enter login
    login_enter = new QLineEdit();
    login_enter->setPlaceholderText("Enter your login");
    login_enter->setFont(appFont);
    login_enter->setMaximumWidth(300);
    login_enter->setAlignment(Qt::AlignHCenter);
    layout->addWidget(login_enter,0,Qt::AlignCenter);

    //Button to create login later login is sent to server that checks if it's okay and sends anwser
    start_button = new QPushButton("Join");
    start_button->setFont(appFont);
    start_button->setEnabled(false); // initially disabled
    start_button->setMaximumWidth(300);
    layout->addWidget(start_button,0,Qt::AlignHCenter);

    setLayout(layout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);


    connect(login_enter, &QLineEdit::textChanged, this, [=](const QString &text){
        start_button->setEnabled(!text.trimmed().isEmpty());
    });

    connect(start_button, &QPushButton::clicked, this, [=]() {
        QString login = login_enter->text();
        emit startClicked(login);
    });
}
