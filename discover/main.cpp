#include <QCoreApplication>
#include <QtDebug>
#include <QTimer>

#include "discovertask.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    DiscoverTask discoverTask;
    QTimer::singleShot(1000, &discoverTask, SLOT(run()));

    return a.exec();
}
