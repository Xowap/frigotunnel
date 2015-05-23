#include "frigoclocktest.h"
#include "frigoclock.h"

FrigoClockTest::FrigoClockTest(QObject *parent) :
    QObject(parent)
{
}

void FrigoClockTest::testPassingTime()
{
    qint64 start = FrigoClock::getTime();
    QTest::qSleep(100);
    qint64 stop = FrigoClock::getTime();

    double diff = (double) (stop - start) / 1000000.0;

    QTRY_VERIFY((int) diff == 100);
}
