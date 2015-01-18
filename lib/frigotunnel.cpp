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
    setupUdp();
    setupTcp();
}

FrigoTunnel::~FrigoTunnel()
{
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
            inboundPacket(packet);
        }

        packet->deleteLater();
    }
}

void FrigoTunnel::inboundPacket(FrigoPacket *packet)
{
    foreach(FrigoMessage *message, packet->getMessages()) {
        if (message->getTargets().contains(name)) {
            if (!uuidSet->contains(message->getUuid())) {
                emit gotMessage(message->getMessage());
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

        inboundPacket(packet);
        packet->deleteLater();

        tcpBuffer = QByteArray(tcpBuffer.data() + headerSize + dataSize, tcpBuffer.size() - headerSize - dataSize);
    }
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

