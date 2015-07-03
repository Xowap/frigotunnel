#ifndef FRIGOPACKET_H
#define FRIGOPACKET_H

#include <QObject>
#include <QList>
#include <QJsonObject>
#include <QByteArray>

#include "frigomessage.h"

class FrigoPacket : public QObject
{
    Q_OBJECT
public:
    explicit FrigoPacket(QObject *parent = 0);
    FrigoPacket(FrigoMessage *message, QObject *parent = 0);
    FrigoPacket(const FrigoMessageList &messages, QObject *parent = 0);
    ~FrigoPacket();

    FrigoPacket *append(FrigoMessage *message);
    FrigoPacket *append(const FrigoMessageList &messages);

    FrigoMessageList getMessages() const;
    qint64 getTime() const;
    qint64 getShift() const;
    void setShift(const qint64 &value);
    qint64 getBaseShift() const;
    void setBaseShift(const qint64 &value);
    QString getSenderId() const;

    int getLatenessMsec();

    QJsonObject toJson() const;
    QByteArray serialize() const;
    QByteArray serializeBinary() const;
    static FrigoPacket *parse(const QByteArray &data, QObject *parent = 0);
    static FrigoPacket *parse(const QJsonObject &obj, QObject *parent = 0);
    static FrigoPacket *parseBinary(const QByteArray &data, QObject *parent = 0);

private:
    FrigoMessageList messages;
    qint64 time;
    qint64 shift;
    qint64 baseShift;
    QString senderId;
};

#endif // FRIGOPACKET_H
