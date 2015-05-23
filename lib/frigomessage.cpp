#include "frigomessage.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <QJsonValue>

FrigoMessage::FrigoMessage(QObject *parent) :
    QObject(parent),
    delay(-1)
{
}

FrigoMessage::FrigoMessage(const QJsonObject &message, QObject *parent) :
    QObject(parent),
    uuid(QUuid::createUuid()),
    message(message),
    system(false),
    delay(-1)
{
}

FrigoMessage::FrigoMessage(const QJsonObject &message, const QStringList targets, QObject *parent) :
    QObject(parent),
    uuid(QUuid::createUuid()),
    message(message),
    targets(targets),
    system(false),
    delay(-1)
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

void FrigoMessage::setSystem(bool system)
{
    this->system = system;
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

bool FrigoMessage::isSystem() const
{
    return system;
}

qint64 FrigoMessage::getDelay() const
{
    return delay;
}

void FrigoMessage::setDelay(const qint64 &value)
{
    delay = value;
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
    json["system"] = isSystem();
    json["delay"] = getDelay();

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
    if (!obj["to"].isArray()
            || !obj["uuid"].isString()
            || !obj["payload"].isObject()
            || !obj["system"].isBool()
            || !obj["delay"].isDouble()) {
        return NULL;
    }

    FrigoMessage *message = new FrigoMessage(parent);

    message->message = obj["payload"].toObject();
    message->uuid = obj["uuid"].toString();
    message->system = obj["system"].toBool();
    message->delay = obj["delay"].toDouble();

    foreach (QJsonValue value, obj["to"].toArray()) {
        if (!value.isString()) {
            return NULL;
        }

        message->targets << value.toString();
    }

    return message;
}
