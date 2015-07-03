#include "frigomessage.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <QJsonValue>
#include <QDataStream>

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

QByteArray FrigoMessage::serializeBinary()
{
    QByteArray out;
    QDataStream ds(&out, QIODevice::WriteOnly);

    ds << (quint8) targets.length();
    foreach (const QString &target, targets) {
        QByteArray targetBytes = target.toUtf8();
        ds << (quint8) targetBytes.length();
        ds.writeRawData(targetBytes.data(), targetBytes.length());
    }

    QByteArray uuidBytes = uuid.toString().toUtf8();
    ds << (quint8) uuidBytes.length();
    ds.writeRawData(uuidBytes.data(), uuidBytes.length());

    ds << isSystem();

    ds << getDelay();

    QByteArray payload = QJsonDocument(getMessage()).toJson(QJsonDocument::Compact);
    ds << (quint8) payload.length();
    ds.writeRawData(payload.data(), payload.length());

    return out;
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

FrigoMessage *FrigoMessage::parseBinary(const QByteArray &data, QObject *parent)
{
    QDataStream ds(data);
    FrigoMessage *message = new FrigoMessage(parent);

    quint8 targetsCount;
    ds >> targetsCount;
    for (int i = 0; i < targetsCount; i += 1) {
        quint8 targetSize;
        ds >> targetSize;
        QByteArray targetBytes(targetSize, '\0');
        ds.readRawData(targetBytes.data(), targetSize);
        message->targets << QString(targetBytes);
    }

    quint8 uuidLength;
    ds >> uuidLength;
    QByteArray uuidBytes(uuidLength, '\0');
    ds.readRawData(uuidBytes.data(), uuidBytes.length());
    message->uuid = QString(uuidBytes);

    ds >> message->system;

    ds >> message->delay;

    quint8 payloadLength;
    ds >> payloadLength;
    QByteArray payload(payloadLength, '\0');
    ds.readRawData(payload.data(), payloadLength);
    message->message = QJsonDocument::fromJson(payload).object();

    return message;
}
