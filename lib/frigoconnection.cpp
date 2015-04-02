#include "frigoconnection.h"
#include "common.h"

#include <QTimer>
#include <QtDebug>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

FrigoConnection::FrigoConnection(QObject *parent) :
    QObject(parent),
    socket(new QTcpSocket(this)),
    errorMatters(false)
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

void FrigoConnection::handleError(QAbstractSocket::SocketError)
{
    if (errorMatters) {
        qDebug() << "Connection lost with" << host;
        QTimer::singleShot(FT_TCP_RETRY_TIMEOUT, this, SLOT(connectSocket()));
    }
}

void FrigoConnection::connectSocket()
{
    errorMatters = false;
    socket->deleteLater();
    socket = new QTcpSocket();

    qDebug() << "Connecting to" << host;

    socket->connectToHost(host, FRIGO_TCP_PORT);
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(handleError(QAbstractSocket::SocketError)));

    int enableKeepAlive = 1;
    int maxIdle = 1 /* seconds */;
    int cntToFail = 1;
    int interval = 1;
    int fd = socket->socketDescriptor();

    setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &enableKeepAlive, sizeof(enableKeepAlive));
    setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &maxIdle, sizeof(maxIdle));
    setsockopt(fd, SOL_TCP, TCP_KEEPCNT, &cntToFail, sizeof(cntToFail));
    setsockopt(fd, SOL_TCP ,TCP_KEEPINTVL, &interval, sizeof(interval));

    QTimer::singleShot(FT_TCP_CONNECT_TIMEOUT, this, SLOT(checkConnected()));
}

void FrigoConnection::checkConnected()
{
    if (socket->state() != QTcpSocket::ConnectedState) {
        qDebug() << "Connection timeout to" << host;
        QTimer::singleShot(FT_TCP_RETRY_TIMEOUT, this, SLOT(connectSocket()));
    } else {
        qDebug() << "Connected to" << host;
        errorMatters = true;
    }
}
