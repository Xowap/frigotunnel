#ifndef DISCOVERTASK_H
#define DISCOVERTASK_H

#include <QObject>

#include "frigotunnel.h"

class DiscoverTask : public QObject
{
    Q_OBJECT
public:
    explicit DiscoverTask(QObject *parent = 0);
    ~DiscoverTask();

public slots:
    void run();

private:
    FrigoTunnel tunnel;
};

#endif // DISCOVERTASK_H
