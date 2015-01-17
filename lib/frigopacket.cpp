#include "frigopacket.h"
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
            || !obj["messages"].isArray()) {
        return NULL;
    }

    FrigoPacket *packet = new FrigoPacket(parent);

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

