#include "frigotunnel.h"
#include "common.h"
#include "frigopacket.h"

#include <QtEndian>
#include <QHostAddress>
#include <QTcpSocket>

FrigoTunnel::FrigoTunnel(QString name, QObject *parent) :
    QObject(parent),
    name(name),
    uuidSet(new ExpiringSet(3, FRIGO_UUID_TTL / 2, this))
{
    connect(this, &FrigoTunnel::gotSystemMessage, this, &FrigoTunnel::inboundSystemMessage);

    setupUdp();
    setupTcp();
}

FrigoTunnel::~FrigoTunnel()
{
}

void FrigoTunnel::send(FrigoPacket *packet, bool skipTcp)
{
    QUdpSocket socket;
    QByteArray data = packet->serialize();
    QHostAddress target(FRIGO_MULTICAST_ADDRESS);
    socket.writeDatagram(data, target, FRIGO_UDP_PORT);
}

void FrigoTunnel::inboundDatagram()
{
    while (udpSocket.hasPendingDatagrams()) {
        QByteArray datagram;
        QHostAddress sender;
        quint16 senderPort;

        datagram.resize(udpSocket.pendingDatagramSize());
        udpSocket.readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        FrigoPacket *packet = FrigoPacket::parse(datagram);

        if (packet != NULL) {
            inboundPacket(packet, sender);
        }

        packet->deleteLater();
    }
}

void FrigoTunnel::inboundPacket(FrigoPacket *packet, const QHostAddress &peer)
{
    foreach(FrigoMessage *message, packet->getMessages()) {
        if (message->getTargets().contains(name) || message->getTargets().contains("*")) {
            if (!uuidSet->contains(message->getUuid())) {
                if (!message->isSystem()) {
                    emit gotMessage(message->getMessage());
                } else {
                    emit gotSystemMessage(message->getMessage(), peer);
                }
            }
        }

        uuidSet->add(message->getUuid());
    }
}

void FrigoTunnel::inboundTcpConnection()
{
    while (tcpServer.hasPendingConnections()) {
        QTcpSocket *connection = tcpServer.nextPendingConnection();
        connect(connection, &QTcpSocket::readyRead, this, &FrigoTunnel::inboundTcpData);
        connect(connection, &QTcpSocket::disconnected, connection, &QTcpSocket::deleteLater);
    }
}

void FrigoTunnel::inboundTcpData()
{
    int headerSize = sizeof(qint32) * 2;
    QTcpSocket *socket = (QTcpSocket *) sender();

    while (socket->bytesAvailable()) {
        tcpBuffer.append(socket->readAll());
    }

    while (tcpBuffer.size() > headerSize) {
        qint32 dataSize, dataSizeCheck;

        dataSize = qFromLittleEndian(*(qint32*)(void*)tcpBuffer.data());
        dataSizeCheck = qFromLittleEndian(*(qint32*)(void*)(tcpBuffer.data() + sizeof(qint32)));

        if (dataSize != (dataSizeCheck ^ FRIGO_TCP_DATA_SIZE_KEY)) {
            tcpBuffer.clear();
            break;
        }

        if (tcpBuffer.size() < (headerSize + dataSize)) {
            break;
        }

        QByteArray data(tcpBuffer.data() + headerSize, dataSize);
        FrigoPacket *packet = FrigoPacket::parse(data);

        if (packet == NULL) {
            continue;
        }

        inboundPacket(packet, socket->peerAddress());
        packet->deleteLater();

        tcpBuffer = QByteArray(tcpBuffer.data() + headerSize + dataSize, tcpBuffer.size() - headerSize - dataSize);
    }
}

void FrigoTunnel::inboundSystemMessage(const QJsonObject &message, const QHostAddress &peer)
{
    if (message["type"] == "say-hello") {
        sayHello();
    } else if (message["type"] == "hello") {
        if (message["name"].isString()) {
            return;
        }

        gotHello(message["name"].toString(), peer);
    }
}

void FrigoTunnel::sayHello()
{
    QJsonObject content;
    content["name"] = name;

    FrigoMessage message(content);
    message.to("*");
    message.setSystem(true);

    FrigoPacket packet(&message);
    send(&packet, true);
}

void FrigoTunnel::gotHello(const QString &name, const QHostAddress &peer)
{
    hosts[name] = peer;
}

void FrigoTunnel::setupUdp()
{
    connect(&udpSocket, SIGNAL(readyRead()), this, SLOT(inboundDatagram()));
    QHostAddress bindAddress(FRIGO_MULTICAST_ADDRESS);
    udpSocket.bind(QHostAddress::AnyIPv4, FRIGO_UDP_PORT);
    udpSocket.joinMulticastGroup(bindAddress);
}

void FrigoTunnel::setupTcp()
{
    tcpServer.listen(QHostAddress::Any, FRIGO_TCP_PORT);
    connect(&tcpServer, &QTcpServer::newConnection, this, &FrigoTunnel::inboundTcpConnection);
}

