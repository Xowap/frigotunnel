#include "frigoclock.h"

#include <QElapsedTimer>

FrigoClock::FrigoClock(QObject *parent) :
    QObject(parent)
{
}

qint64 FrigoClock::getTime()
{
    static QElapsedTimer timer;
    static bool started = false;

    if (!started) {
        started = true;
        timer.start();
    }

    return timer.nsecsElapsed();
}
