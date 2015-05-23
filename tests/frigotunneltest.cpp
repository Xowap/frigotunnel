#include "frigotunneltest.h"
#include "frigotunnel.h"
#include "common.h"

#include <QtTest/QtTest>
#include <QtDebug>
#include <QtEndian>
#include <QSignalSpy>
#include <QJsonObject>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QElapsedTimer>

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
    int count = 0;

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

    tunnel.inboundPacket(&packet, QHostAddress());
    tunnel.inboundPacket(&packet, QHostAddress());

    QCOMPARE(youpi, QString("tralala"));
    QCOMPARE(count, 1);
}

void FrigoTunnelTest::delayedPacketPropagation()
{
    FrigoTunnel *tunnel = new FrigoTunnel("test");
    QJsonObject messageContent;
    QSignalSpy messageSpy(tunnel, SIGNAL(gotMessage(QJsonObject)));

    FrigoMessage message(messageContent);
    message.to("*");
    message.setDelay(100);

    FrigoPacket packet(&message);
    tunnel->inboundPacket(&packet, QHostAddress());

    QCOMPARE(messageSpy.count(), 0);

    QTest::qWait(95);
    QCOMPARE(messageSpy.count(), 0);

    QTest::qWait(10);
    QCOMPARE(messageSpy.count(), 1);

    tunnel->deleteLater();
}

void FrigoTunnelTest::udpReception()
{
    FrigoTunnel tunnel("test");
    QSignalSpy messageSpy(&tunnel, SIGNAL(gotMessage(QJsonObject)));
    QJsonObject messageContent;
    FrigoMessage message(messageContent);
    message.to("test");

    FrigoPacket packet(&message);

    QUdpSocket socket;
    QByteArray data = packet.serialize();
    QHostAddress target(FRIGO_MULTICAST_ADDRESS);
    socket.writeDatagram(data, target, FRIGO_UDP_PORT);

    QTRY_VERIFY(messageSpy.count() == 1);
}

void FrigoTunnelTest::tcpReception()
{
    FrigoTunnel tunnel("test");
    QSignalSpy messageSpy(&tunnel, SIGNAL(gotMessage(QJsonObject)));
    QJsonObject messageContent;
    FrigoMessage message(messageContent);
    message.to("test");

    FrigoPacket packet(&message);

    QTcpSocket socket;
    QSignalSpy connectSpy(&socket, SIGNAL(connected()));
    socket.connectToHost("localhost", FRIGO_TCP_PORT);

    QTRY_VERIFY(connectSpy.count() == 1);

    QByteArray data = packet.serialize();
    qint32 size = qToLittleEndian(data.size()), sizeCheck = qToLittleEndian(size ^ FRIGO_TCP_DATA_SIZE_KEY);

    socket.write((char*)(void*) &size, sizeof(qint32));
    socket.write((char*)(void*) &sizeCheck, sizeof(qint32));
    QTRY_VERIFY(tunnel.tcpBuffer.size() == (int) sizeof(qint32) * 2);

    socket.write(data.left(10));
    QTRY_VERIFY(tunnel.tcpBuffer.size() == ((int) sizeof(qint32) * 2) + 10);

    socket.write(data.right(data.size() - 10));
    socket.close();

    QTRY_VERIFY(messageSpy.count() == 1);
}

