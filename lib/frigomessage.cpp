#include "frigomessage.h"

#include <QJsonDocument>
#include <QJsonArray>

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

