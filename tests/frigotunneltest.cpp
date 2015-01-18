#include "frigotunneltest.h"
#include "frigotunnel.h"
#include "common.h"

#include <QtTest/QtTest>
#include <QtDebug>
#include <QJsonObject>
#include <QUdpSocket>

FrigoTunnelTest::FrigoTunnelTest(QObject *parent) : QObject(parent)
{

}

FrigoTunnelTest::~FrigoTunnelTest()
{

}

void FrigoTunnelTest::packetPropagation()
{
    FrigoTunnel tunnel("test");
    QJsonObject messageContent;
    QString youpi;
    int count;

    messageContent["youpi"] = "tralala";
    FrigoMessage message(messageContent);
    message.to("youpi");
    message.to("pouet");
    message.to("test");

    FrigoPacket packet(&message);

    connect(&tunnel, &FrigoTunnel::gotMessage, [&](QJsonObject receivedMessage) {
        youpi = receivedMessage["youpi"].toString();
        count += 1;
    });

    tunnel.inboundPacket(&packet);
    tunnel.inboundPacket(&packet);

    QCOMPARE(youpi, QString("tralala"));
    QCOMPARE(count, 1);
}

void FrigoTunnelTest::udpReception()
{
    int count;
    FrigoTunnel tunnel("test");
    QJsonObject messageContent;
    FrigoMessage message(messageContent);
    message.to("test");

    FrigoPacket packet(&message);

    connect(&tunnel, &FrigoTunnel::gotMessage, [&](QJsonObject) {
        count += 1;
    });

    QUdpSocket socket;
    QByteArray data = packet.serialize();
    QHostAddress target(FRIGO_MULTICAST_ADDRESS);
    socket.writeDatagram(data, target, FRIGO_UDP_PORT);

    QTRY_COMPARE(count, 1);
}

