#include <QPainter>
#include "hangman.h"

Hangman::Hangman(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(120, 200);
}

void Hangman::setMistakes(int count)
{
    mistakes = count;
    update(); // triggers repaint
}

void Hangman::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(Qt::black, 3));

    // base
    if (mistakes >= 1)
        p.drawLine(20, height()-20, 100, height()-20);

    // pole
    if (mistakes >= 2)
        p.drawLine(60, height()-20, 60, 20);

    // beam
    if (mistakes >= 3)
        p.drawLine(60, 20, 120, 20);

    // rope
    if (mistakes >= 4)
        p.drawLine(120, 20, 120, 40);

    // head
    if (mistakes >= 5)
        p.drawEllipse(QPoint(120, 55), 15, 15);

    // body
    if (mistakes >= 6)
        p.drawLine(120, 70, 120, 110);

    // arms
    if (mistakes >= 7)
        p.drawLine(120, 80, 100, 95);
    if (mistakes >= 8)
        p.drawLine(120, 80, 140, 95);

    // legs
    if (mistakes >= 9)
        p.drawLine(120, 110, 100, 140);
    if (mistakes >= 10)
        p.drawLine(120, 110, 140, 140);
}
