#include "sperogui.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    speroGui w;
    w.show();
    return a.exec();
}
