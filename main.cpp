#include "sctcpserverwidget.h"

#include <QApplication>
#include <QDebug>

class A
{
    int a;
    int b;
    long c;
    short d;
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qDebug() << sizeof(A);
    SCTcpServerWidget w;
    w.show();
    return a.exec();
}
