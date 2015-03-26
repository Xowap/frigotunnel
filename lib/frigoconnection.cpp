#include "frigoconnection.h"
#include "common.h"

#include <QTimer>
#include <QtDebug>

FrigoConnection::FrigoConnection(TimeoutGenerator *timeoutGenerator, QObject *parent) :
    QObject(parent),
    socket(new QTcpSocket()),
    timeoutGenerator(timeoutGenerator)
{
}

FrigoConnection::~FrigoConnection()
{
}

void FrigoConnection::setHost(const QHostAddress &host)
{
    if (this->host != host) {
        this->host = host;
        connectSocket();
    }
}

const QHostAddress FrigoConnection::getHost()
{
    return host;
}

void FrigoConnection::write(const QByteArray &data)
{
    socket->write(data);
}

void FrigoConnection::handleDisconnect()
{
    int timeout = timeoutGenerator->generate();
    QTimer::singleShot(timeout, this, SLOT(connectSocket()));
}

void FrigoConnection::connectSocket()
{
    socket->deleteLater();
    socket = new QTcpSocket();

    qDebug() << "Connecting to" << host;

    socket->connectToHost(host, FRIGO_TCP_PORT);
    connect(socket, &QTcpSocket::disconnected, this, &FrigoConnection::handleDisconnect);
}
