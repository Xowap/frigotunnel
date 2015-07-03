#include "frigopacket.h"
#include "frigoclock.h"
#include "frigotunnel.h"
#include "common.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QtEndian>
#include <QDataStream>

FrigoPacket::FrigoPacket(QObject *parent) :
    QObject(parent)
{
}

FrigoPacket::FrigoPacket(FrigoMessage *message, QObject *parent) :
    QObject(parent)
{
    messages.append(message);
}

FrigoPacket::FrigoPacket(const FrigoMessageList &messages, QObject *parent) :
    QObject(parent),
    messages(messages)
{
}

FrigoPacket::~FrigoPacket()
{
}

FrigoPacket *FrigoPacket::append(FrigoMessage *message)
{
    messages << message;
    return this;
}

FrigoPacket *FrigoPacket::append(const FrigoMessageList &messages)
{
    this->messages.append(messages);
    return this;
}

FrigoMessageList FrigoPacket::getMessages() const
{
    return messages;
}

qint64 FrigoPacket::getTime() const
{
    return time;
}

qint64 FrigoPacket::getShift() const
{
    return shift;
}

void FrigoPacket::setShift(const qint64 &value)
{
    shift = value;
}

qint64 FrigoPacket::getBaseShift() const
{
    return baseShift;
}

void FrigoPacket::setBaseShift(const qint64 &value)
{
    baseShift = value;
}

QString FrigoPacket::getSenderId() const
{
    return senderId;
}

int FrigoPacket::getLatenessMsec()
{
    return (shift - baseShift) / 1000000;
}

QJsonObject FrigoPacket::toJson() const
{
    QJsonObject json;
    QJsonArray messages;

    foreach(FrigoMessage *message, this->messages) {
        messages << message->toJson();
    }

    json["protocol"] = "frigo";
    json["version"] = FRIGO_PROTOCOL_VERSION;
    json["messages"] = messages;
    json["sender-id"] = FrigoTunnel::getSenderId();
    json["time"] = QString::number(FrigoClock::getTime());

    return json;
}

QByteArray FrigoPacket::serialize() const
{
    return QJsonDocument(toJson()).toJson();
}

QByteArray FrigoPacket::serializeBinary() const
{
    QByteArray out;
    QDataStream ds(&out, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::LittleEndian);

    ds << (quint8) FRIGO_PROTOCOL_VERSION;

    QByteArray senderBytes = FrigoTunnel::getSenderId().toUtf8();
    ds << (quint8) senderBytes.length();
    ds.writeRawData(senderBytes.data(), senderBytes.length());

    ds << FrigoClock::getTime();

    ds << (quint8) messages.length();
    foreach (FrigoMessage *message, messages) {
        QByteArray messageBytes = message->serializeBinary();

        if (messageBytes.length() <= 0xff) {
            ds << (quint8) messageBytes.length();
            ds.writeRawData(messageBytes.data(), messageBytes.length());
        }
    }

    return out;
}

FrigoPacket *FrigoPacket::parse(const QByteArray &data, QObject *parent)
{
    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &jsonError);

    if (jsonError.error != QJsonParseError::NoError || !doc.isObject()) {
        return NULL;
    }

    return parse(doc.object(), parent);
}

FrigoPacket *FrigoPacket::parse(const QJsonObject &obj, QObject *parent)
{
    if (obj["protocol"] != "frigo"
            || obj["version"] != FRIGO_PROTOCOL_VERSION
            || !obj["messages"].isArray()
            || !obj["time"].isString()
            || !obj["sender-id"].isString()) {
        return NULL;
    }

    FrigoPacket *packet = new FrigoPacket(parent);

    bool timeOk;
    packet->time = obj["time"].toString().toLongLong(&timeOk);

    if (!timeOk) {
        return NULL;
    }

    packet->senderId = obj["sender-id"].toString();

    foreach(QJsonValue value, obj["messages"].toArray()) {
        if (!value.isObject()) {
            return NULL;
        }

        FrigoMessage *message = FrigoMessage::parse(value.toObject(), packet);

        if (message == NULL) {
            return NULL;
        }

        packet->messages << message;
    }

    return packet;
}

FrigoPacket *FrigoPacket::parseBinary(const QByteArray &data, QObject *parent)
{
    QDataStream ds(data);
    FrigoPacket *packet = new FrigoPacket(parent);

    ds.setByteOrder(QDataStream::LittleEndian);

    quint8 version;
    ds >> version;

    if (version != FRIGO_PROTOCOL_VERSION) {
        return NULL;
    }

    quint8 senderSize;
    ds >> senderSize;
    QByteArray senderBytes(senderSize, '\0');
    ds.readRawData(senderBytes.data(), senderSize);
    packet->senderId = QString(senderBytes);

    ds >> packet->time;

    quint8 msgCount;
    ds >> msgCount;

    for (int i = 0; i < msgCount; i += 1) {
        quint8 msgSize;
        ds >> msgSize;
        QByteArray msgBytes(msgSize, '\0');
        ds.readRawData(msgBytes.data(), msgSize);

        FrigoMessage *message = FrigoMessage::parseBinary(msgBytes, packet);

        if (message == NULL) {
            return NULL;
        }

        packet->messages << message;
    }

    return packet;
}
