#include "frigomessage.h"

FrigoMessage::FrigoMessage(const QJsonObject &message, QObject *parent) :
    QObject(parent),
    message(message)
{

}

FrigoMessage::FrigoMessage(const QJsonObject &message, const QStringList targets, QObject *parent) :
    QObject(parent),
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

