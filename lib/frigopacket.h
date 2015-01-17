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
    explicit FrigoPacket(QObject *parent = 0);
    FrigoPacket(FrigoMessage *message, QObject *parent = 0);
    FrigoPacket(const FrigoMessageList &messages, QObject *parent = 0);
    ~FrigoPacket();

    FrigoPacket *append(FrigoMessage *message);
    FrigoPacket *append(const FrigoMessageList &messages);

    FrigoMessageList getMessages() const;

    QJsonObject toJson() const;
    QByteArray serialize() const;
    static FrigoPacket *parse(const QByteArray &data, QObject *parent = 0);
    static FrigoPacket *parse(const QJsonObject &obj, QObject *parent = 0);

private:
    FrigoMessageList messages;
};

#endif // FRIGOPACKET_H
