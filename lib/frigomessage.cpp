#include "frigomessage.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <QJsonValue>

FrigoMessage::FrigoMessage(QObject *parent) :
    QObject(parent)
{
}

FrigoMessage::FrigoMessage(const QJsonObject &message, QObject *parent) :
    QObject(parent),
    uuid(QUuid::createUuid()),
    message(message)
{
}

FrigoMessage::FrigoMessage(const QJsonObject &message, const QStringList targets, QObject *parent) :
    QObject(parent),
    uuid(QUuid::createUuid()),
    message(message),
    targets(targets)
{
}

FrigoMessage::~FrigoMessage()
{

}

void FrigoMessage::to(QString target)
{
    targets.append(target);
}

void FrigoMessage::to(QStringList targets)
{
    this->targets.append(targets);
}

QJsonObject FrigoMessage::getMessage() const
{
    return message;
}

QStringList FrigoMessage::getTargets() const
{
    return targets;
}

QString FrigoMessage::getUuid() const
{
    return uuid.toString();
}

QJsonObject FrigoMessage::toJson()
{
    QJsonObject json;
    QJsonArray jsonTargets;

    foreach(QString target, targets) {
        jsonTargets << target;
    }

    json["to"] = jsonTargets;
    json["uuid"] = uuid.toString();
    json["payload"] = getMessage();

    return json;
}

QByteArray FrigoMessage::serialize()
{
    return QJsonDocument(toJson()).toJson();
}

FrigoMessage *FrigoMessage::parse(const QByteArray &data, QObject *parent)
{
    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &jsonError);

    if (jsonError.error != QJsonParseError::NoError || !doc.isObject()) {
        return NULL;
    }

    return parse(doc.object(), parent);
}

FrigoMessage *FrigoMessage::parse(const QJsonObject &obj, QObject *parent)
{
    if (!obj["to"].isArray() || !obj["uuid"].isString() || !obj["payload"].isObject()) {
        return NULL;
    }

    FrigoMessage *message = new FrigoMessage(parent);

    message->message = obj["payload"].toObject();
    message->uuid = obj["uuid"].toString();

    foreach (QJsonValue value, obj["to"].toArray()) {
        if (!value.isString()) {
            return NULL;
        }

        message->targets << value.toString();
    }

    return message;
}

