#include "parsepackettest.h"
#include "frigopacket.h"
#include "expiringset.h"

#include <QFile>

ParsePacketTest::ParsePacketTest(QObject *parent) : QObject(parent)
{

}

ParsePacketTest::~ParsePacketTest()
{

}

void ParsePacketTest::testParseValidPacket()
{
    QFile f(":/tests/packet1.json");

    QCOMPARE(f.open(QFile::ReadOnly), true);

    FrigoPacket *packet = FrigoPacket::parse(f.readAll());

    QCOMPARE(packet->getMessages().length(), 1);

    FrigoMessage *message = packet->getMessages()[0];
    QStringList expectTo = QStringList() << "test1" << "test2";

    QCOMPARE(message->getTargets(), expectTo);
}

void ParsePacketTest::testParseInvalidPacket()
{    QFile f(":/tests/packet2.json");

     QCOMPARE(f.open(QFile::ReadOnly), true);

     FrigoPacket *packet = FrigoPacket::parse(f.readAll());

      QVERIFY(packet == NULL);
}

void ParsePacketTest::testSenderFollowUp()
{
    QFile f(":/tests/packet1.json");

    QCOMPARE(f.open(QFile::ReadOnly), true);

    FrigoPacket *packet = FrigoPacket::parse(f.readAll());

    QCOMPARE(packet->getSenderId(), QString("super-sender"));
}
