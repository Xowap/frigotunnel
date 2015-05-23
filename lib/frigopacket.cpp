#include "frigopacket.h"
#include "frigoclock.h"
#include "frigotunnel.h"
#include "common.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>

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
