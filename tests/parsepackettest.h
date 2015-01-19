#ifndef PARSEPACKETTEST_H
#define PARSEPACKETTEST_H

#include <QObject>
#include <QtTest/QtTest>

class ParsePacketTest : public QObject
{
    Q_OBJECT
public:
    explicit ParsePacketTest(QObject *parent = 0);
    ~ParsePacketTest();

private slots:
    void testParseValidPacket();
    void testParseInvalidPacket();
};

#endif // PARSEPACKETTEST_H
