#include "discovertask.h"
#include "frigotunnel.h"

#include <QUuid>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTextStream>
#include <QCoreApplication>

#include <cstdio>

DiscoverTask::DiscoverTask(QObject *parent) :
    QObject(parent),
    tunnel(QUuid::createUuid().toString())
{
}

DiscoverTask::~DiscoverTask()
{

}

void DiscoverTask::run()
{
    QJsonObject obj;

    for(ConnectionMap::const_iterator i = tunnel.getConnections().constBegin(); i != tunnel.getConnections().constEnd(); i++) {
        obj[i.key()] = i.value()->getHost().toString();
    }

    QJsonDocument doc(obj);

    QTextStream out(stdout);
    out << doc.toJson();

    QCoreApplication::exit();
}
