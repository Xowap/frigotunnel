#ifndef FRIGOCONNECTION_H
#define FRIGOCONNECTION_H

#include <QObject>
#include <QHostAddress>
#include <QTcpSocket>
#include <QByteArray>

#include "timeoutgenerator.h"

#define FT_TCP_RETRY_TIMEOUT 1000
#define FT_TCP_CONNECT_TIMEOUT 1000

class FrigoConnection : public QObject
{
    Q_OBJECT
public:
    explicit FrigoConnection(QObject *parent = 0);
    ~FrigoConnection();

    void setHost(const QHostAddress &host);
    const QHostAddress getHost();
    void write(const QByteArray &data);

private slots:
    void handleError(QAbstractSocket::SocketError);
    void connectSocket();
    void checkConnected();

private:
    QHostAddress host;
    QTcpSocket *socket;
    bool errorMatters;
};

#endif // FRIGOCONNECTION_H
