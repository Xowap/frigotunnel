#ifndef FRIGOCLOCKTEST_H
#define FRIGOCLOCKTEST_H

#include <QObject>
#include <QtTest/QtTest>

class FrigoClockTest : public QObject
{
    Q_OBJECT
public:
    explicit FrigoClockTest(QObject *parent = 0);

private slots:
    void testPassingTime();
};

#endif // FRIGOCLOCKTEST_H
