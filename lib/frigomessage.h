#ifndef FRIGOMESSAGE_H
#define FRIGOMESSAGE_H

#include <QObject>
#include <QJsonObject>
#include <QStringList>
#include <QList>
#include <QByteArray>
#include <QUuid>

class FrigoMessage : public QObject
{
    Q_OBJECT
public:
    FrigoMessage(const QJsonObject &message, QObject *parent = 0);
    FrigoMessage(const QJsonObject &message, const QStringList targets, QObject *parent = 0);
    ~FrigoMessage();

    void to(QString target);
    void to(QStringList targets);

    QJsonObject getMessage() const;
    QStringList getTargets() const;

    QJsonObject toJson();
    QByteArray serialize();

private:
    QUuid uuid;
    QJsonObject message;
    QStringList targets;
};

typedef QList<FrigoMessage *> FrigoMessageList;

#endif // FRIGOMESSAGE_H
