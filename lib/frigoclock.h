#ifndef FRIGOCLOCK_H
#define FRIGOCLOCK_H

#include <QObject>

class FrigoClock : public QObject
{
    Q_OBJECT
private:
    explicit FrigoClock(QObject *parent = 0);

public:
    static qint64 getTime();
};

#endif // FRIGOCLOCK_H
