#include "common.h"
#include "frigoconnectiontest.h"
#include "frigoconnection.h"

#include <QtTest/QtTest>
#include <QTcpServer>

FrigoConnectionTest::FrigoConnectionTest(QObject *parent) :
    QObject(parent),
    timeoutGenerator(new TimeoutGenerator(10, 20, this))
{
}

FrigoConnectionTest::~FrigoConnectionTest()
{

}

void FrigoConnectionTest::testConnect()
{
    QTcpServer server;
    server.listen(QHostAddress::Any, FRIGO_TCP_PORT);

    FrigoConnection connection(timeoutGenerator);
    connection.setHost(QHostAddress::LocalHost);

    QTRY_VERIFY(server.hasPendingConnections());
}

void FrigoConnectionTest::testReconnect()
{
    QTcpServer server;
    server.listen(QHostAddress::Any, FRIGO_TCP_PORT);

    FrigoConnection connection(timeoutGenerator);
    connection.setHost(QHostAddress::LocalHost);

    QTRY_VERIFY(server.hasPendingConnections());

    QTcpSocket *socket = server.nextPendingConnection();
    socket->close();

    QTRY_VERIFY_WITH_TIMEOUT(server.hasPendingConnections(), 10000);
}
