#ifndef HANGMANWIDGET_H
#define HANGMANWIDGET_H

#include <QWidget>

class Hangman : public QWidget
{
    Q_OBJECT

public:
    explicit Hangman(QWidget *parent = nullptr);
    void setMistakes(int count);
    int getMistakes() const { return mistakes; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int mistakes = 0;
};

#endif
