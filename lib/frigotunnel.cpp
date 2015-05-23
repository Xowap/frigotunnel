#include "frigotunnel.h"
#include "common.h"
#include "frigopacket.h"
#include "frigoclock.h"

#include <QtEndian>
#include <QHostAddress>
#include <QTcpSocket>
#include <QTimer>

FrigoTunnel::FrigoTunnel(QString name, QObject *parent) :
    QObject(parent),
    name(name),
    uuidSet(new ExpiringSet(3, FRIGO_UUID_TTL / 2, this)),
    udpSocket(new QUdpSocket(this)),
    timeoutGenerator(new TimeoutGenerator(FRIGO_TCP_RECONNECT_MIN, FRIGO_TCP_RECONNECT_MAX, this))
{
    connect(this, &FrigoTunnel::gotSystemMessage, this, &FrigoTunnel::inboundSystemMessage);

    setupUdp();
    setupTcp();
    askHello();

    connect(&helloTimer, &QTimer::timeout, this, &FrigoTunnel::sayHello);
    helloTimer.setInterval(FRIGO_HELLO_INTERVAL_MSEC);
    helloTimer.start();
}

FrigoTunnel::~FrigoTunnel()
{
}

void FrigoTunnel::send(FrigoPacket *packet, bool skipTcp, int udpSends)
{
    QUdpSocket socket;
    QByteArray data = packet->serialize();
    QHostAddress target(FRIGO_MULTICAST_ADDRESS);

    for (int i; i < udpSends; i += 1) {
        socket.writeDatagram(data, target, FRIGO_UDP_PORT);
    }

    qDebug() << "Sending packet..." << skipTcp << udpSends;

    if (!skipTcp) {
        for(ConnectionMap::iterator i = connections.begin(); i != connections.end(); i++) {
            (*i)->write(data);
            qDebug() << " ... to" << i.value()->getHost();
        }
    }
}

const ConnectionMap FrigoTunnel::getConnections()
{
    return connections;
}

QString FrigoTunnel::getSenderId()
{
    static QString senderId = QUuid::createUuid().toString();
    return senderId;
}

void FrigoTunnel::inboundDatagram()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        QHostAddress sender;
        quint16 senderPort;

        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        FrigoPacket *packet = FrigoPacket::parse(datagram);

        if (packet != NULL) {
            inboundPacket(packet, sender);
        }

        packet->deleteLater();
    }
}

void FrigoTunnel::inboundPacket(FrigoPacket *packet, QHostAddress peer)
{
    accountShift(packet, peer);
    int lateness = packet->getLatenessMsec();

    foreach(FrigoMessage *message, packet->getMessages()) {
        if (message->getTargets().contains(name) || message->getTargets().contains("*")) {
            if (!uuidSet->contains(message->getUuid())) {
                qDebug() << "Packet in! " + QString::number(lateness) + "ms late.";

                QJsonObject jsonMessage = message->getMessage();
                bool isSystem = message->isSystem();

                auto propagateMessage = [=]() {
                    if (!isSystem) {
                        emit gotMessage(jsonMessage);
                    } else {
                        emit gotSystemMessage(jsonMessage, peer);
                    }
                };

                if (message->getDelay() < 0) {
                    propagateMessage();
                } else if (lateness <= message->getDelay()) {
                    QTimer *timer = new QTimer();
                    timer->setSingleShot(true);
                    timer->setTimerType(Qt::PreciseTimer);
                    connect(timer, &QTimer::timeout, [=]() {
                        propagateMessage();
                        timer->deleteLater();
                    });
                    timer->start(message->getDelay() - lateness);
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
        if (!message["name"].isString()) {
            return;
        }

        gotHello(message["name"].toString(), peer);
    }
}

void FrigoTunnel::askHello()
{
    QJsonObject content;
    content["type"] = "say-hello";

    FrigoMessage message(content);
    message.to("*");
    message.setSystem(true);

    FrigoPacket packet(&message);
    send(&packet, true);
}

void FrigoTunnel::sayHello()
{
    QJsonObject content;
    content["type"] = "hello";
    content["name"] = name;

    FrigoMessage message(content);
    message.to("*");
    message.setSystem(true);

    FrigoPacket packet(&message);
    send(&packet, true, 1);

    qDebug() << "Said Hello";
}

void FrigoTunnel::gotHello(const QString &name, const QHostAddress &peer)
{
    if (!peer.isLoopback()) {
        QStringList toDelete;

        for(ConnectionMap::iterator i = connections.begin(); i != connections.end(); i++) {
            if (i.value()->getHost() == peer && i.key() != name) {
                i.value()->deleteLater();
                toDelete << i.key();
            }
        }

        for (auto key : toDelete) {
            connections.remove(key);
        }

        if (!connections.contains(name)) {
            connections[name] = new FrigoConnection(this);
        }

        connections[name]->setHost(peer);
    }
}

void FrigoTunnel::setupUdp()
{
    bindUdp();
}

void FrigoTunnel::bindUdp()
{
    QHostAddress bindAddress(FRIGO_MULTICAST_ADDRESS);

    udpSocket->deleteLater();
    udpSocket = new QUdpSocket(this);

    qDebug() << "Binding UDP socket to" << bindAddress;
    udpSocket->bind(QHostAddress::AnyIPv4, FRIGO_UDP_PORT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    if (udpSocket->state() == QUdpSocket::BoundState) {
        qDebug() << "Joining multicast group" << bindAddress;
        udpSocket->leaveMulticastGroup(bindAddress);
        udpSocket->joinMulticastGroup(bindAddress);
    } else {
        qWarning() << "UDP SOCKET IS NOT BOUND AND I HAVE NO CLUE WHY";
    }

    QTimer::singleShot(timeoutGenerator->generate(), this, SLOT(bindUdp()));
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(inboundDatagram()));
}

void FrigoTunnel::setupTcp()
{
    tcpServer.listen(QHostAddress::Any, FRIGO_TCP_PORT);
    connect(&tcpServer, &QTcpServer::newConnection, this, &FrigoTunnel::inboundTcpConnection);
}

void FrigoTunnel::accountShift(FrigoPacket *packet, const QHostAddress &peer)
{
    qint64 shift = FrigoClock::getTime() - packet->getTime();
    QString remoteId = peer.toString() + "/" + packet->getSenderId();

    if (!shifts.contains(remoteId)) {
        shifts[remoteId] = shift;
    }

    if (shifts[remoteId] > shift) {
        shifts[remoteId] = shift;
    }

    packet->setShift(shift);
    packet->setBaseShift(shifts[remoteId]);
}
