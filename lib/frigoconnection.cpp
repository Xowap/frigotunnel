#include "frigoconnection.h"
#include "common.h"

#include <QTimer>

FrigoConnection::FrigoConnection(TimeoutGenerator *timeoutGenerator, QObject *parent) :
    QObject(parent),
    timeoutGenerator(timeoutGenerator)
{
    connect(&socket, &QTcpSocket::disconnected, this, &FrigoConnection::handleDisconnect);
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

void FrigoConnection::write(const QByteArray &data)
{
    socket.write(data);
}

void FrigoConnection::handleDisconnect()
{
    int timeout = timeoutGenerator->generate();
    QTimer::singleShot(timeout, this, SLOT(connectSocket()));
}

void FrigoConnection::connectSocket()
{
    if (socket.isOpen()) {
        socket.close();
    }

    socket.connectToHost(host, FRIGO_TCP_PORT);
}
