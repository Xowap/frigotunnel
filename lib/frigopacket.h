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
    FrigoPacket(FrigoMessage *message, QObject *parent = 0);
    FrigoPacket(const FrigoMessageList &messages, QObject *parent = 0);
    ~FrigoPacket();

    FrigoPacket *append(FrigoMessage *message);
    FrigoPacket *append(const FrigoMessageList &messages);

    FrigoPacket *operator <<(FrigoMessage *message);
    FrigoPacket *operator <<(const FrigoMessageList &messages);

    QJsonObject toJson() const;
    QByteArray serialize() const;

private:
    FrigoMessageList messages;
};

#endif // FRIGOPACKET_H
