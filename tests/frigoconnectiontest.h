#ifndef FRIGOCONNECTIONTEST_H
#define FRIGOCONNECTIONTEST_H

#include <QObject>

#include "timeoutgenerator.h"

class FrigoConnectionTest : public QObject
{
    Q_OBJECT
public:
    explicit FrigoConnectionTest(QObject *parent = 0);
    ~FrigoConnectionTest();

private slots:
    void testConnect();
    void testReconnect();

private:
    TimeoutGenerator *timeoutGenerator;
};

#endif // FRIGOCONNECTIONTEST_H
