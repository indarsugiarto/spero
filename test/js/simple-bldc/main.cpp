
#include <QtCore/QCoreApplication>
#include <QtCore/QObject>
#include "gamepadmonitor.h"

int main(int argc, char **argv)
{
    QCoreApplication application(argc, argv);

    GamepadMonitor monitor;
    QObject::connect(&monitor, SIGNAL(finished()), &application, SLOT(quit()), Qt::QueuedConnection);

    return application.exec();
}
