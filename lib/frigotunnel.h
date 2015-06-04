#ifndef FRIGOTUNNEL_H
#define FRIGOTUNNEL_H

#include <QObject>
#include <QUdpSocket>
#include <QTcpServer>
#include <QJsonObject>
#include <QByteArray>
#include <QMap>
#include <QTimer>

#include "frigopacket.h"
#include "expiringset.h"
#include "frigoconnection.h"

class FrigoTunnelTest;

typedef QMap<QString, FrigoConnection*> ConnectionMap;
typedef QMap<QString, qint64> ShiftMap;

class FrigoTunnel : public QObject
{
    Q_OBJECT

    friend class FrigoTunnelTest;

public:
    FrigoTunnel(QString name, QObject *parent = 0);
    ~FrigoTunnel();

    void send(FrigoPacket *packet, bool skipTcp = false, int udpSends = 5);
    const ConnectionMap getConnections();

    static QString getSenderId(bool regenerate = false);

private slots:
    void inboundDatagram();
    void inboundPacket(FrigoPacket *packet, QHostAddress peer);
    void inboundTcpConnection();
    void inboundTcpData();
    void inboundSystemMessage(const QJsonObject &message, const QHostAddress &peer);

    void askHello();
    void sayHello();
    void gotHello(const QString &name, const QHostAddress &peer);
    void bindUdp();
    void checkClock();

private:
    static QString makeSenderId();

signals:
    void gotMessage(const QJsonObject &message);
    void gotSystemMessage(const QJsonObject &message, const QHostAddress &peer);

private:
    QString name;
    ExpiringSet *uuidSet;
    QUdpSocket *udpSocket;
    QTcpServer tcpServer;
    QByteArray tcpBuffer;
    ConnectionMap connections;
    TimeoutGenerator *timeoutGenerator;
    QTimer helloTimer, clockTimer;
    ShiftMap shifts;
    QString senderId;

    void setupUdp();
    void setupTcp();

    void accountShift(FrigoPacket *packet, const QHostAddress &peer);
};

#endif // FRIGOTUNNEL_H
