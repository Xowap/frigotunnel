#ifndef FRIGOTUNNEL_H
#define FRIGOTUNNEL_H

#include <QObject>
#include <QUdpSocket>
#include <QJsonObject>

#include "frigopacket.h"
#include "expiringset.h"

class FrigoTunnelTest;

class FrigoTunnel : public QObject
{
    Q_OBJECT

    friend class FrigoTunnelTest;

public:
    FrigoTunnel(QString name, QObject *parent = 0);
    ~FrigoTunnel();

private slots:
    void inboundDatagram();
    void inboundPacket(FrigoPacket *packet);

signals:
    void gotMessage(const QJsonObject &message);

private:
    QString name;
    ExpiringSet *uuidSet;
    QUdpSocket udpSocket;

    void setupUdp();
};

#endif // FRIGOTUNNEL_H
