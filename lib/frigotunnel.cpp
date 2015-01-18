#include "frigotunnel.h"
#include "common.h"
#include "frigopacket.h"

#include <QHostAddress>

FrigoTunnel::FrigoTunnel(QString name, QObject *parent) :
    QObject(parent),
    name(name),
    uuidSet(new ExpiringSet(3, FRIGO_UUID_TTL / 2, this))
{
    setupUdp();
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

void FrigoTunnel::setupUdp()
{
    connect(&udpSocket, SIGNAL(readyRead()), this, SLOT(inboundDatagram()));
    QHostAddress bindAddress(FRIGO_MULTICAST_ADDRESS);
    udpSocket.bind(QHostAddress::AnyIPv4, FRIGO_UDP_PORT);
    udpSocket.joinMulticastGroup(bindAddress);
}

