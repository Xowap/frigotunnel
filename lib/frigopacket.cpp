#include "frigopacket.h"

#include <QJsonArray>
#include <QJsonDocument>

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

FrigoPacket *FrigoPacket::operator <<(FrigoMessage *message)
{
    return append(message);
}

FrigoPacket *FrigoPacket::operator <<(const FrigoMessageList &messages)
{
    return append(messages);
}

QJsonObject FrigoPacket::toJson() const
{
    QJsonObject json;
    QJsonArray messages;

    foreach(FrigoMessage *message, this->messages) {
        messages << message->toJson();
    }

    json["protocol"] = "frigo";
    json["version"] = 1;
    json["messages"] = messages;

    return json;
}

QByteArray FrigoPacket::serialize() const
{
    return QJsonDocument(toJson()).toJson();
}

