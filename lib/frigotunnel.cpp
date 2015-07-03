#include "frigotunnel.h"
#include "common.h"
#include "frigopacket.h"
#include "frigoclock.h"

#include <QtEndian>
#include <QHostAddress>
#include <QTcpSocket>
#include <QTimer>
#include <QDateTime>
#include <QNetworkInterface>

#ifndef Q_OS_ANDROID
#include <QtSerialPort/QtSerialPort>
#endif

FrigoTunnel::FrigoTunnel(const QString &name, const QStringList &radioDevices, QObject *parent) :
    QObject(parent),
    name(name),
    uuidSet(new ExpiringSet(3, FRIGO_UUID_TTL / 2, this)),
    udpSocket(new QUdpSocket(this)),
    timeoutGenerator(new TimeoutGenerator(FRIGO_TCP_RECONNECT_MIN, FRIGO_TCP_RECONNECT_MAX, this)),
    radioSocket(NULL),
    radioDevices(radioDevices),
    radioSending(false)
{
    connect(this, &FrigoTunnel::gotSystemMessage, this, &FrigoTunnel::inboundSystemMessage);

    setupUdp();
    setupTcp();
    setupRadio();
    askHello();

    connect(&helloTimer, &QTimer::timeout, this, &FrigoTunnel::sayHello);
    helloTimer.setInterval(FRIGO_HELLO_INTERVAL_MSEC);
    helloTimer.start();

    connect(&clockTimer, &QTimer::timeout, this, &FrigoTunnel::checkClock);
    clockTimer.setInterval(FRIGO_HELLO_INTERVAL_MSEC);
    clockTimer.start();
}

FrigoTunnel::~FrigoTunnel()
{
}

void FrigoTunnel::send(FrigoPacket *packet, bool skipTcp, int udpSends)
{
//    QUdpSocket socket;
    QByteArray data = packet->serialize();
//    QHostAddress target(FRIGO_MULTICAST_ADDRESS);

//    for (int i = 0; i < udpSends; i += 1) {
//        socket.writeDatagram(data, target, FRIGO_UDP_PORT);
//    }

    sendRadio(data);

    qDebug() << "Sending packet..." << skipTcp << udpSends;

//    if (!skipTcp) {
//        for(ConnectionMap::iterator i = connections.begin(); i != connections.end(); i++) {
//            (*i)->write(data);
//            qDebug() << " ... to" << i.value()->getHost();
//        }
//    }
}

const ConnectionMap FrigoTunnel::getConnections()
{
    return connections;
}

QString FrigoTunnel::getSenderId(bool regenerate)
{
    static QString senderId = makeSenderId();

    if (regenerate) {
        senderId = makeSenderId();
    }

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

        // Delete the packet in 10s, when nobody needs it anymore
        QTimer::singleShot(10000, packet, SLOT(deleteLater()));
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

        // Delete the packet in 10s, when nobody needs it anymore
        QTimer::singleShot(10000, packet, SLOT(deleteLater()));

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

void FrigoTunnel::inboundRadioData()
{
    static QByteArray buf;
    int headerSize = sizeof(qint16) * 2 + sizeof(quint32);
    int i;

    while (radioSocket->bytesAvailable()) {
        buf.append(radioSocket->readAll());
    }

    for (i = 0; i < buf.length(); i += 1) {
        if (buf.at(i) != 0) {
            break;
        }
    }

    if (i > 0) {
        buf = QByteArray(buf.data() + i, buf.size() - i);
    }

    while (buf.size() > headerSize) {
        qint16 dataSize, dataSizeCheck;
        quint32 ipInt;

        dataSize = qFromLittleEndian(*(qint16*)(void*)buf.data());
        dataSizeCheck = qFromLittleEndian(*(qint16*)(void*)(buf.data() + sizeof(qint16)));
        ipInt = qFromLittleEndian(*(quint32*)(void*)(buf.data() + sizeof(qint16) * 2));

        if (dataSize != (dataSizeCheck ^ FRIGO_TCP_DATA_SIZE_KEY)) {
            buf = QByteArray(buf.data() + 1, buf.size() - 1);
            continue;
        }

        if (buf.size() < (headerSize + dataSize)) {
            break;
        }

        QByteArray data(buf.data() + headerSize, dataSize);
        FrigoPacket *packet = FrigoPacket::parse(data);

        buf.clear();

        if (packet != NULL) {
            inboundPacket(packet, QHostAddress(ipInt));
        }
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

void FrigoTunnel::checkClock()
{
    static qint64 lastDiff = 0;
    qint64 diff = (FrigoClock::getTime() / 1000000) - QDateTime::currentMSecsSinceEpoch();

    qDebug() << "diff" << diff << lastDiff;

    if (qAbs(diff - lastDiff) > 1000) {
        getSenderId(true);
        shifts.clear();
        qDebug() << "CLOCK RESET";
    }

    lastDiff = diff;
}

QString FrigoTunnel::makeSenderId()
{
    return QUuid::createUuid().toString();
}

void FrigoTunnel::setupTcp()
{
    tcpServer.listen(QHostAddress::Any, FRIGO_TCP_PORT);
    connect(&tcpServer, &QTcpServer::newConnection, this, &FrigoTunnel::inboundTcpConnection);
}

void FrigoTunnel::setupRadio()
{
#ifndef Q_OS_ANDROID
    QSerialPort *port = new QSerialPort(this);

    foreach (const QString &device, radioDevices) {
        port->setPortName(device);
        port->setBaudRate(19200);
        port->setDataBits(QSerialPort::Data8);
        port->setStopBits(QSerialPort::OneStop);
        port->setParity(QSerialPort::NoParity);

        if (port->open(QSerialPort::ReadWrite)) {
            radioSocket = port;
            break;
        }
    }
#endif

    if (radioSocket == NULL) {
        QTcpSocket *tcpPort = new QTcpSocket(this);
        tcpPort->connectToHost(QHostAddress::LocalHost, FRIGO_RADIO_PORT);
        radioSocket = tcpPort;
    }

    connect(radioSocket, &QIODevice::readyRead, this, &FrigoTunnel::inboundRadioData);

    localIp = QHostAddress(QHostAddress::LocalHost);

    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol
                && address != QHostAddress(QHostAddress::LocalHost)) {
            localIp = address;
        }
    }
}

void FrigoTunnel::sendRadio(const QByteArray &data)
{
     if (radioSocket != NULL && radioSocket->isWritable()) {
         QByteArray realData;

         qint16 size = qToLittleEndian(data.size()), sizeCheck = qToLittleEndian(size ^ FRIGO_TCP_DATA_SIZE_KEY);
         quint32 ip = qToLittleEndian(localIp.toIPv4Address());

         realData.append((char*)(void*) &size, sizeof(qint16));
         realData.append((char*)(void*) &sizeCheck, sizeof(qint16));
         realData.append((char*)(void*) &ip, sizeof(quint32));
         realData.append(data);

         if (sendQueue.size() > FRIGO_RADIO_QUEUE_SIZE) {
             sendQueue.clear();
         }

         if (realData.size() > FRIGO_RADIO_PACKET_SIZE) {
             qDebug() << "BEWARE, Radio Fragment!" << realData.size();
         }

         for (int i = 0; i < realData.size(); i += FRIGO_RADIO_PACKET_SIZE) {
             sendQueue.enqueue(QByteArray(realData.data() + i, qMin(FRIGO_RADIO_PACKET_SIZE, realData.size() - i)));
         }

         if (!radioSending) {
             sendNextRadioPacket();
         }
     }
}

void FrigoTunnel::sendNextRadioPacket()
{
    radioSending = true;

    if (!sendQueue.isEmpty()) {
        QByteArray data = sendQueue.dequeue();
        radioSocket->write(data);
        QTimer::singleShot(500, this, SLOT(sendNextRadioPacket()));
    } else {
        radioSending = false;
    }
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
