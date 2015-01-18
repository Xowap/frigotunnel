#include "timeoutgeneratortest.h"
#include "timeoutgenerator.h"

#include <QtTest/QtTest>
#include <QtDebug>

TimeoutGeneratorTest::TimeoutGeneratorTest(QObject *parent) : QObject(parent)
{
}

TimeoutGeneratorTest::~TimeoutGeneratorTest()
{
}

void TimeoutGeneratorTest::generateRandom()
{
    TimeoutGenerator generator(0, 10000);
    int min = 10000, max = 0;

    for (int i = 0; i < 100; i += 1) {
        int val = generator.generate();

        if (val < min) {
            min = val;
        }

        if (val > max) {
            max = val;
        }
    }

    QVERIFY(min < max);
}
