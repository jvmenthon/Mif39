//#include "mainwindow.h"
#include <QApplication>
#include <QPushButton>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QWidget w;
    w.setFixedSize(300,150);
    QPushButton start("Commencer le test",&w);
    start.move(50,50);
    w.show();

    return a.exec();
}
