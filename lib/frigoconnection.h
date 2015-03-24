#ifndef FRIGOCONNECTION_H
#define FRIGOCONNECTION_H

#include <QObject>
#include <QHostAddress>
#include <QTcpSocket>
#include <QByteArray>

#include "timeoutgenerator.h"

class FrigoConnection : public QObject
{
    Q_OBJECT
public:
    explicit FrigoConnection(TimeoutGenerator *timeoutGenerator, QObject *parent = 0);
    ~FrigoConnection();

    void setHost(const QHostAddress &host);
    const QHostAddress getHost();
    void write(const QByteArray &data);

private slots:
    void handleDisconnect();
    void connectSocket();

private:
    QHostAddress host;
    QTcpSocket socket;
    TimeoutGenerator *timeoutGenerator;
};

#endif // FRIGOCONNECTION_H
