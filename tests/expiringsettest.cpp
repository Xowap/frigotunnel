#include "expiringsettest.h"

ExpiringSetTest::ExpiringSetTest(QObject *parent) : QObject(parent)
{

}

ExpiringSetTest::~ExpiringSetTest()
{

}

void ExpiringSetTest::calcCycleAfter()
{
    ExpiringSet set(5);

    QCOMPARE(set.cycleAfter(0), 1);
    QCOMPARE(set.cycleAfter(1), 2);
    QCOMPARE(set.cycleAfter(2), 3);
    QCOMPARE(set.cycleAfter(3), 4);
    QCOMPARE(set.cycleAfter(4), 0);
}

void ExpiringSetTest::setFunctionning()
{
    ExpiringSet set(1);
    set.add("test");
    QVERIFY(set.contains("test"));
}

void ExpiringSetTest::expiration()
{
    ExpiringSet set(3);

    set.cycle();
    set.add("test");
    QVERIFY(set.contains("test"));

    set.cycle();
    set.cycle();
    QVERIFY(set.contains("test"));

    set.cycle();
    QVERIFY(!set.contains("test"));
}

void ExpiringSetTest::autoExpiration()
{
    ExpiringSet set(3, 1);

    set.add("test");

    QVERIFY(set.contains("test"));
    QTRY_VERIFY(!set.contains("test"));
}

