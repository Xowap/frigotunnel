#ifndef EXPIRINGSETTEST_H
#define EXPIRINGSETTEST_H

#include <QObject>
#include <QtTest/QtTest>

#include "expiringset.h"

class ExpiringSetTest : public QObject
{
    Q_OBJECT
public:
    explicit ExpiringSetTest(QObject *parent = 0);
    ~ExpiringSetTest();

private slots:
    void calcCycleAfter();
    void setFunctionning();
    void expiration();
};

#endif // EXPIRINGSETTEST_H
