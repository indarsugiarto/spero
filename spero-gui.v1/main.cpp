#include "wmain.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    wMain w;
    w.show();
    return a.exec();
}
