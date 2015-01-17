#include "expiringset.h"

ExpiringSet::ExpiringSet(int cycles, QObject *parent) :
    ExpiringSet(cycles, 0, parent)
{
}

ExpiringSet::ExpiringSet(int cycles, int period, QObject *parent) :
    QObject(parent),
    cycles(cycles),
    currentCycle(0),
    period(period),
    buffer(ExpiringSetBuffer(cycles))
{
    if (period > 0) {
        connect(&cycleTimer, SIGNAL(timeout()), this, SLOT(cycle()));
        cycleTimer.setInterval(period);
        cycleTimer.start();
    }
}

ExpiringSet::~ExpiringSet()
{
}

void ExpiringSet::add(QString key)
{
    buffer[currentCycle] << key;
}

bool ExpiringSet::contains(QString key)
{
    foreach (QSet<QString> subSet, buffer) {
        if (subSet.contains(key)) {
            return true;
        }
    }

    return false;
}

void ExpiringSet::cycle()
{
    int cycleAfterValue = cycleAfter(currentCycle);
    buffer[cycleAfterValue].clear();
    currentCycle = cycleAfterValue;
}

int ExpiringSet::cycleAfter(int cycle)
{
    return (cycle + 1) % cycles;
}
